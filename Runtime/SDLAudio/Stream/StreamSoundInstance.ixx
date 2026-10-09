module;

#include "SDL3/SDL.h"
#include "vorbis/vorbisfile.h"

export module SDLAudio:StreamSoundInstance;

import std;
import :Device;
export import :StreamSoundBuffer;
import :VorbisDecoder;
import Core.Utils;
import Contract.Audio.Interfaces;

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

export class StreamSoundInstance : public ISoundInstance
{
public:
	StreamSoundInstance() = default;

	~StreamSoundInstance() override
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

	bool Setup(AudioDevice* device)
	{
		m_device = device;
		std::memset(&m_vorbisFile, 0, sizeof(OggVorbis_File));
		return true;
	}

	bool SetBuffer(StreamSoundBuffer* buffer)
	{
		if (m_state == PlaybackState::Playing)
			return false;
		if (buffer == nullptr)
			return false;

		m_buffer = buffer;
		return true;
	}

	bool Reset(const PlaybackParams& params) override
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

	bool Play() override
	{
		if (!m_stream)
			return false;

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

	bool Pause() override
	{
		if (m_state != PlaybackState::Playing)
			return false;
		if (!m_stream)
			return false;

		m_state = PlaybackState::Paused;
		return true;
	}

	bool Resume() override
	{
		if (m_state != PlaybackState::Paused)
			return false;
		if (!m_stream)
			return false;

		m_state = PlaybackState::Playing;
		return SetVolume(m_volume);
	}

	bool Stop() override
	{
		if (!m_stream)
			return false;

		if (!SDL_ClearAudioStream(m_stream)) return false;

		m_draining = false;
		m_state = PlaybackState::Stopped;

		return true;
	}

	void Update() override
	{
		if (!m_stream || m_state != PlaybackState::Playing)
			return;

		if (!m_draining)
		{
			constexpr int LOW_WATERMARK = 128 * 1024;

			while (SDL_GetAudioStreamQueued(m_stream) < LOW_WATERMARK)
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

	bool SetVolume(float volume) override
	{
		if (!SDL_SetAudioStreamGain(m_stream, volume)) return false;
		m_volume = volume;

		return true;
	}

	PlaybackState GetState() const noexcept override
	{
		if (!m_stream)
			return Core::InvalidEnum<PlaybackState>;
		return m_state;
	}

private:
	bool PrepareStream()
	{
		m_resourceStream = m_buffer->GetStream()->Clone();

		if (m_vorbisOpened)
		{
			ov_clear(&m_vorbisFile);
			m_vorbisOpened = false;
		}

		auto cb = Vorbis::CreateCallbacks();
		int result = ov_open_callbacks(m_resourceStream.get(), &m_vorbisFile, nullptr, 0, cb);
		if (result < 0)
			return false;
		m_vorbisOpened = true;

		SDL_AudioSpec srcSpec = VorbisToSDLAudioSpec(m_vorbisFile);

		if (m_stream)
			SDL_DestroyAudioStream(m_stream);
		m_stream = m_device->CreateDeviceStream(srcSpec);

		return m_stream != nullptr;
	}

	bool PushChunk()
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

	AudioDevice* m_device{ nullptr };
	StreamSoundBuffer* m_buffer{ nullptr };
	std::unique_ptr<IReadStream> m_resourceStream;
	OggVorbis_File m_vorbisFile{};
	bool m_vorbisOpened{ false };

	SDL_AudioStream* m_stream{ nullptr };
	float m_volume{ 1.0f };
	bool m_loop{ false };
	bool m_draining{ false };
	PlaybackState m_state{ PlaybackState::Stopped };

	std::array<char, 16384> m_decodeBuffer{};
};