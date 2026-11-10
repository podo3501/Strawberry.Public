export module Contract.Audio.Backend:IAudioBackend;

import std;
export import :ISoundBuffer;
export import :ISoundInstance;

export struct IAudioBackend
{
	virtual ~IAudioBackend() = default;

	virtual bool Initialize(int maxVoices, int maxStreams) noexcept = 0;
	virtual std::unique_ptr<ISoundBuffer> CreateStaticSoundBuffer() = 0;
	virtual std::unique_ptr<ISoundBuffer> CreateStreamSoundBuffer() = 0;
	// 빈 인스턴스를 빌려준다(없으면 nullptr). 빌린 쪽이 Stop을 부를 때까지 다른 요청에 주지 않는다.
	virtual ISoundInstance* RequestStaticInstance(ISoundBuffer* sndBuffer) = 0;
	virtual ISoundInstance* RequestStreamInstance(ISoundBuffer* sndBuffer) = 0;
};

export std::unique_ptr<IAudioBackend> CreateAudioBackend();