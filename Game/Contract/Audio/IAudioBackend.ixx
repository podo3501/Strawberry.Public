export module Client.Audio.Interfaces:IAudioBackend;

import std;
export import :ISoundBuffer;
export import :ISoundInstance;

export struct IAudioBackend
{
	virtual ~IAudioBackend() = default;

	virtual bool Initialize(int maxVoices, int maxStreams) noexcept = 0;
	virtual std::unique_ptr<ISoundBuffer> CreateStaticSoundBuffer() = 0;
	virtual std::unique_ptr<ISoundBuffer> CreateStreamSoundBuffer() = 0;
	virtual ISoundInstance* RequestStaticInstance(ISoundBuffer* sndBuffer) = 0;
	virtual ISoundInstance* RequestStreamInstance(ISoundBuffer* sndBuffer) = 0;
};

export std::unique_ptr<IAudioBackend> CreateAudioBackend();