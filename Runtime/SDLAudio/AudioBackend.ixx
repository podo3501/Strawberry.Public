module;

#ifdef _DEBUG
#pragma comment(lib, "SDL3-static_Debug.lib")
#pragma comment(lib, "SDL3_mixer-static_Debug.lib")
#else
#pragma comment(lib, "SDL3-static_Release.lib")
#pragma comment(lib, "SDL3_mixer-static_Release.lib")
#endif

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "imm32.lib")

export module SDLAudio:Backend;

import std;
import :Device;
import :Mixer;
import :StaticSoundInstance;
import :StreamSoundInstance;
import Contract.Audio.Interfaces;

class SDLAudioBackend : public IAudioBackend
{
public:
	SDLAudioBackend() = default;
	~SDLAudioBackend() override = default;

	bool Initialize(int maxVoices, int maxStreams) noexcept override
	{
		if (maxVoices > VoiceLimits::MaxVoices || maxStreams > VoiceLimits::MaxStreams)
			return false;
		if (maxVoices < maxStreams)
			return false;

		if (!m_mixer.Initialize()) return false;
		if (!m_streamDevice.Initialize(StreamAudioRequestDevice{})) return false;
		if (!SetupStaticInstances(maxVoices)) return false;
		if (!SetupStreamInstances(maxStreams)) return false;

		return true;
	}

	std::unique_ptr<ISoundBuffer> CreateStaticSoundBuffer() override
	{
		return std::make_unique<StaticSoundBuffer>(m_mixer.Get());
	}

	std::unique_ptr<ISoundBuffer> CreateStreamSoundBuffer() override
	{
		return std::make_unique<StreamSoundBuffer>();
	}

	ISoundInstance* RequestStaticInstance(ISoundBuffer* sndBuffer) override
	{
		auto staticBuffer = static_cast<StaticSoundBuffer*>(sndBuffer);
		for (auto& instance : m_staticInstances)
		{
			if (instance.SetBuffer(staticBuffer))
				return &instance;
		}

		return nullptr;
	}

	ISoundInstance* RequestStreamInstance(ISoundBuffer* sndBuffer) override
	{
		auto streamBuffer = static_cast<StreamSoundBuffer*>(sndBuffer);
		for (auto& instance : m_streamInstances)
		{
			if (instance->SetBuffer(streamBuffer))
				return instance.get();
		}

		return nullptr;
	}

private:
	bool SetupStaticInstances(int maxVoices) noexcept
	{
		m_staticInstances.resize(maxVoices);
		for (auto& instance : m_staticInstances)
			if (!instance.Setup(m_mixer.Get())) return false;

		return true;
	}

	bool SetupStreamInstances(int maxStreams) noexcept
	{
		m_streamInstances.reserve(maxStreams);
		for (int i = 0; i < maxStreams; ++i)
		{
			auto instance = std::make_unique<StreamSoundInstance>();
			if (!instance->Setup(&m_streamDevice))
				return false;

			m_streamInstances.emplace_back(std::move(instance));
		}

		return true;
	}

	AudioMixer m_mixer;
	AudioDevice m_streamDevice;
	std::vector<StaticSoundInstance> m_staticInstances;
	std::vector<std::unique_ptr<StreamSoundInstance>> m_streamInstances;
};

std::unique_ptr<IAudioBackend> CreateAudioBackend()
{
	return std::make_unique<SDLAudioBackend>();
}