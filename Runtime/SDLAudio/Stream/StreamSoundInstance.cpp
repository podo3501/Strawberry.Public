module;

#include "SDL3/SDL.h"
#include "vorbis/vorbisfile.h"

module SDLAudio;

import :StreamSoundInstance;
import :VorbisDecoder;
import Core.Utils;
import Core.Logger;

static SDL_AudioSpec VorbisToSDLAudioSpec(OggVorbis_File& vf)
{
	SDL_AudioSpec spec{};
	if (auto* vi = ov_info(&vf, -1))
	{
		spec.channels = static_cast<Uint8>(vi->channels);
		spec.freq = static_cast<int>(vi->rate);
	}
	else
	{
		spec.channels = 2;
		spec.freq = 44100;
	}
	spec.format = SDL_AUDIO_S16; // ov_read()는 항상 signed 16bit PCM을 반환

	return spec;
}

StreamSoundInstance::~StreamSoundInstance()
{
	if (m_stream)
	{
		SDL_DestroyAudioStream(m_stream);
		m_stream = nullptr;
	}

	if (m_vorbisOpened)
	{
		ov_clear(&m_vorbisFile);
	}
}

bool StreamSoundInstance::Setup(AudioDevice* device)
{
	m_device = device;
	std::memset(&m_vorbisFile, 0, sizeof(OggVorbis_File));
	return true;
}

bool StreamSoundInstance::SetBuffer(StreamSoundBuffer* buffer)
{
	if (m_state == PlaybackState::Playing)
		return false;
	if (buffer == nullptr)
		return false;

	m_buffer = buffer;
	return true;
}

bool StreamSoundInstance::Reset(const PlaybackParams& params)
{
	if (!PrepareStream()) return false;

	ov_pcm_seek(&m_vorbisFile, 0);
	SDL_ClearAudioStream(m_stream);
	if (!SetVolume(params.volume)) return false;

	m_loop = params.loop;
	m_draining = false;
	m_state = PlaybackState::Stopped;

	return true;
}

bool StreamSoundInstance::Play()
{
	if (!m_stream)
		return false;

	if (!EnsureBound()) return false;

	m_draining = false;
	m_state = PlaybackState::Playing;

	constexpr int INITIAL_FILL = 4;
	for (int i = 0; i < INITIAL_FILL; ++i)
	{
		if (!PushChunk())
		{
			if (!m_draining)
				return false; // 데이터를 다 넣고 끝난게 아니라면 오류.
			break;            // 소리 버퍼가 원래 작은 것.
		}
	}

	return true;
}

bool StreamSoundInstance::Pause()
{
	if (m_state != PlaybackState::Playing)
		return false;
	if (!m_stream)
		return false;

	SDL_UnbindAudioStream(m_stream); // 쌓인 소리는 스트림에 남기고 장치가 가져가지 못하게 해서 바로 조용해진다.
	m_state = PlaybackState::Paused;
	return true;
}

bool StreamSoundInstance::Resume()
{
	if (m_state != PlaybackState::Paused)
		return false;
	if (!m_stream)
		return false;
	if (!EnsureBound()) return false;

	m_state = PlaybackState::Playing;
	return SetVolume(m_volume);
}

bool StreamSoundInstance::EnsureBound()
{
	if (SDL_GetAudioStreamDevice(m_stream) != 0)
		return true;

	if (!SDL_BindAudioStream(m_device->Get(), m_stream))
	{
		Core::Logger::Error("StreamSoundInstance: 오디오 스트림을 장치에 다시 연결하지 못함: {}", SDL_GetError());
		return false;
	}

	return true;
}

void StreamSoundInstance::Stop()
{
	if (!m_stream)
		return; // Reset 전이라 멈출 것이 없다.

	if (!SDL_ClearAudioStream(m_stream))
		Core::Logger::Error("StreamSoundInstance: 오디오 스트림을 비우지 못함: {}", SDL_GetError());

	m_draining = false;
	m_state = PlaybackState::Stopped;
}

void StreamSoundInstance::Update()
{
	if (!m_stream || m_state != PlaybackState::Playing)
		return;

	if (!m_draining)
	{
		while (SDL_GetAudioStreamQueued(m_stream) < m_lowWatermarkBytes)
		{
			if (!PushChunk())
				break;
		}
	}
	else
	{
		if (SDL_GetAudioStreamQueued(m_stream) == 0)
		{
			m_draining = false;
			m_state = PlaybackState::Stopped;
		}
	}
}

bool StreamSoundInstance::SetVolume(float volume)
{
	if (!SDL_SetAudioStreamGain(m_stream, volume)) return false;
	m_volume = volume;

	return true;
}

PlaybackState StreamSoundInstance::GetState() const noexcept
{
	if (!m_stream)
		return Core::InvalidEnum<PlaybackState>;
	return m_state;
}


bool StreamSoundInstance::PrepareStream()
{
	// 기존 상태를 건드리기 전에 필요한 것이 다 있는지 먼저 확인한다.
	if (!m_device)
	{
		Core::Logger::Error("StreamSoundInstance: 장치가 지정되지 않음 (Setup 필요)");
		return false;
	}
	if (!m_buffer)
	{
		Core::Logger::Error("StreamSoundInstance: 버퍼가 지정되지 않음 (SetBuffer 필요)");
		return false;
	}
	IReadStream* source = m_buffer->GetStream();
	if (!source)
	{
		Core::Logger::Error("StreamSoundInstance: 버퍼에 스트림이 없음 (LoadFromAsset 필요)");
		return false;
	}
	auto resourceStream = source->Clone();
	if (!resourceStream)
	{
		Core::Logger::Error("StreamSoundInstance: 스트림을 복제하지 못함");
		return false;
	}
	m_resourceStream = std::move(resourceStream);

	if (m_vorbisOpened)
	{
		ov_clear(&m_vorbisFile);
		m_vorbisOpened = false;
	}

	auto cb = Vorbis::CreateCallbacks();
	int result = ov_open_callbacks(m_resourceStream.get(), &m_vorbisFile, nullptr, 0, cb);
	if (result < 0)
	{
		Core::Logger::Error("StreamSoundInstance: Ogg Vorbis 데이터를 열지 못함 ({})", result);
		return false;
	}
	m_vorbisOpened = true;

	SDL_AudioSpec srcSpec = VorbisToSDLAudioSpec(m_vorbisFile);

	constexpr double PREFETCH_SECONDS = 0.5;
	const double bytesPerSecond = static_cast<double>(srcSpec.freq) * srcSpec.channels * sizeof(std::int16_t); // 디코딩 결과는 항상 S16
	m_lowWatermarkBytes = static_cast<int>(bytesPerSecond * PREFETCH_SECONDS);

	if (m_stream)
		SDL_DestroyAudioStream(m_stream);
	m_stream = m_device->CreateDeviceStream(srcSpec);

	return m_stream != nullptr;
}

bool StreamSoundInstance::PushChunk()
{
	if (!m_stream || !m_vorbisOpened)
		return false;

	int bitstream = 0;
	long bytes = ov_read(&m_vorbisFile, m_decodeBuffer.data(), static_cast<int>(m_decodeBuffer.size()),
		0, // little endian
		2, // 16bit PCM
		1, // signed
		&bitstream);

	if (bytes > 0)
	{
		SDL_PutAudioStreamData(m_stream, m_decodeBuffer.data(), bytes);
		return true;
	}

	if (bytes == 0)
	{
		if (m_loop)
		{
			if (ov_pcm_seek(&m_vorbisFile, 0) == 0)
				return true;
		}
		else
		{
			m_draining = true;
			SDL_FlushAudioStream(m_stream);
		}

		return false;
	}

	// bytes < 0 (디코딩 오류)
	m_draining = false;
	m_state = PlaybackState::Stopped;
	SDL_ClearAudioStream(m_stream);

	return false;
}