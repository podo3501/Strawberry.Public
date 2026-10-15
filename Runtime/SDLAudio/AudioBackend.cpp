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

module SDLAudio;

bool AudioBackend::Initialize(int maxVoices, int maxStreams) noexcept
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

std::unique_ptr<ISoundBuffer> AudioBackend::CreateStaticSoundBuffer()
{
	return std::make_unique<StaticSoundBuffer>(m_mixer.Get());
}

std::unique_ptr<ISoundBuffer> AudioBackend::CreateStreamSoundBuffer()
{
	return std::make_unique<StreamSoundBuffer>();
}

ISoundInstance* AudioBackend::RequestStaticInstance(ISoundBuffer* sndBuffer)
{
	auto staticBuffer = static_cast<StaticSoundBuffer*>(sndBuffer);
	for (auto& instance : m_staticInstances)
	{
		if (instance.SetBuffer(staticBuffer))
			return &instance;
	}

	return nullptr;
}

ISoundInstance* AudioBackend::RequestStreamInstance(ISoundBuffer* sndBuffer)
{
	auto streamBuffer = static_cast<StreamSoundBuffer*>(sndBuffer);
	for (auto& instance : m_streamInstances)
	{
		if (instance->SetBuffer(streamBuffer))
			return instance.get();
	}

	return nullptr;
}

bool AudioBackend::SetupStaticInstances(int maxVoices) noexcept
{
	m_staticInstances.resize(maxVoices);
	for (auto& instance : m_staticInstances)
		if (!instance.Setup(m_mixer.Get())) return false;

	return true;
}

bool AudioBackend::SetupStreamInstances(int maxStreams) noexcept
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

std::unique_ptr<IAudioBackend> CreateAudioBackend()
{
	return std::make_unique<AudioBackend>();
}