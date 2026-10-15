module;

#include "SDL3/SDL.h"

export module SDLAudio:Device;

import std;
import :Config;

export class AudioDevice
{
public:
	~AudioDevice();
	AudioDevice() = default;
	bool Initialize(const StreamAudioRequestDevice& config);

	SDL_AudioStream* CreateDeviceStream(const SDL_AudioSpec& srcSpec) const;
	SDL_AudioDeviceID Get() const noexcept;

private:
	SDL_AudioDeviceID m_device{};
	SDL_AudioSpec m_spec{};
};