module;

#include "SDL3_mixer/SDL_mixer.h"

export module SDLAudio:Mixer;

export class AudioMixer //RAII를 위해서 작은 클래스로 만듦.
{
public:
	AudioMixer() = default;
	AudioMixer(const AudioMixer&) = delete;
	AudioMixer& operator=(const AudioMixer&) = delete;

	~AudioMixer()
	{
		if (m_mixer)
		{
			MIX_DestroyMixer(m_mixer);
		}
		MIX_Quit();
		SDL_Quit();
	}

	bool Initialize()
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

	MIX_Mixer* Get() const noexcept
	{
		return m_mixer;
	}

private:
	MIX_Mixer* m_mixer{ nullptr }; //믹서 안에 Device가 들어가 있다. 
};