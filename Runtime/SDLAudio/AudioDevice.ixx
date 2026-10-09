module;

#include "SDL3/SDL.h"

export module SDLAudio:Device;

import std;
import :Config;

export class AudioDevice
{
public:
	AudioDevice() = default;

	~AudioDevice()
	{
		if (m_device != 0)
		{
			SDL_CloseAudioDevice(m_device);
		}
	}

	bool Initialize(const StreamAudioRequestDevice& config)
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

	SDL_AudioStream* CreateDeviceStream(const SDL_AudioSpec& srcSpec) const
	{
		auto stream = SDL_CreateAudioStream(&srcSpec, &m_spec);
		if (!SDL_BindAudioStream(m_device, stream))
			return nullptr;

		return stream;
	}

	SDL_AudioDeviceID Get() const noexcept
	{
		return m_device;
	}

private:
	SDL_AudioDeviceID m_device{};
	SDL_AudioSpec m_spec{};
};