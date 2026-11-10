module;

#include "SDL3/SDL.h"

module SDLAudio;

import :Device;
import Core.Logger;

AudioDevice::~AudioDevice()
{
	if (m_device != 0)
	{
		SDL_CloseAudioDevice(m_device);
	}
}

bool AudioDevice::Initialize(const StreamAudioRequestDevice& config)
{
	SDL_AudioSpec desired{};
	desired.freq = config.freq;
	desired.format = config.format;
	desired.channels = config.channels;

	m_device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired);
	if (m_device == 0)
		return false;

	return SDL_GetAudioDeviceFormat(m_device, &m_spec, nullptr);
}

SDL_AudioStream* AudioDevice::CreateDeviceStream(const SDL_AudioSpec& srcSpec) const
{
	SDL_AudioStream* stream = SDL_CreateAudioStream(&srcSpec, &m_spec);
	if (!stream)
	{
		Core::Logger::Error("AudioDevice: 오디오 스트림을 만들지 못함: {}", SDL_GetError());
		return nullptr;
	}

	if (!SDL_BindAudioStream(m_device, stream))
	{
		Core::Logger::Error("AudioDevice: 오디오 스트림을 장치에 연결하지 못함: {}", SDL_GetError());
		SDL_DestroyAudioStream(stream);   // 연결하지 못한 스트림은 호출한 쪽이 받지 못하므로 여기서 해제한다
		return nullptr;
	}

	return stream;
}

SDL_AudioDeviceID AudioDevice::Get() const noexcept
{
	return m_device;
}