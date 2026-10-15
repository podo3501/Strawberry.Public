export module SDLAudio;

import std;
import :Device;
import :Mixer;
import :StaticSoundInstance;
import :StreamSoundInstance;
import Contract.Audio.Interfaces;

class AudioBackend : public IAudioBackend
{
public:
	AudioBackend() = default;
	~AudioBackend() override = default;
	bool Initialize(int maxVoices, int maxStreams) noexcept override;

	std::unique_ptr<ISoundBuffer> CreateStaticSoundBuffer() override;
	std::unique_ptr<ISoundBuffer> CreateStreamSoundBuffer() override;

	ISoundInstance* RequestStaticInstance(ISoundBuffer* sndBuffer) override;
	ISoundInstance* RequestStreamInstance(ISoundBuffer* sndBuffer) override;

private:
	bool SetupStaticInstances(int maxVoices) noexcept;
	bool SetupStreamInstances(int maxStreams) noexcept;

	AudioMixer m_mixer;
	AudioDevice m_streamDevice;
	std::vector<StaticSoundInstance> m_staticInstances;
	std::vector<std::unique_ptr<StreamSoundInstance>> m_streamInstances;
};