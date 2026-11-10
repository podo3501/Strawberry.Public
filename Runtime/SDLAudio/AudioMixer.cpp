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
	if (m_libraryReady)
	{
		MIX_Quit();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		if (SDL_WasInit(0) == 0)
			SDL_Quit();   // 켜진 것이 하나도 없으면 SDL이 잡은 전역 정보까지 해제한다
	}
}

bool AudioMixer::Initialize()
{
	if (m_mixer)
		return true;

	if (!m_libraryReady)
	{
		// SDL은 초기화 횟수를 세므로 믹서마다 한 번씩 켜고 소멸할 때 한 번씩 끈다.
		if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) return false;
		if (!MIX_Init())
		{
			SDL_QuitSubSystem(SDL_INIT_AUDIO);
			return false;
		}
		m_libraryReady = true;
	}

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