module;

#include "SDL3/SDL_audio.h"

export module SDLAudio:Config;

export struct VoiceLimits
{
	static constexpr int MaxVoices = 64;
	static constexpr int MaxStreams = 16;
};

export struct StreamAudioRequestDevice
{
	int freq = 48000;
	SDL_AudioFormat format = SDL_AUDIO_F32;
	int channels = 2;
};