module;

#include "SDL3_mixer/SDL_mixer.h"

module SDLAudio;

import :Mixer;

AudioMixer::~AudioMixer()
{
	if (m_mixer)
	{
		MIX_DestroyMixer(m_mixer);
	}
	MIX_Quit();
	SDL_Quit();
}

bool AudioMixer::Initialize()
{
	bool isInit = (SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) != 0;
	if (isInit)
		return true;

	if (!SDL_Init(SDL_INIT_AUDIO)) return false;
	if (!MIX_Init()) return false;

	SDL_AudioSpec spec{}; //static sound 설정부분.
	spec.freq = 48000;
	spec.format = SDL_AUDIO_S16;
	spec.channels = 2;

	m_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
	return m_mixer != nullptr;
}

MIX_Mixer* AudioMixer::Get() const noexcept
{
	return m_mixer;
}