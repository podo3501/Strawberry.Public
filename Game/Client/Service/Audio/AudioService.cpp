module Service.Audio;

import :AudioService;
import :LoadedSound;
import Core.Utils;
import Contract.Asset.Data;

static void ApplyStaticParams(const StaticSoundDesc* desc, PlaybackParams& params) noexcept
{
	// Static 전용 파라미터 적용 지점
}

static void ApplyStreamParams(const StreamSoundDesc* desc, PlaybackParams& params) noexcept
{
	params.loop = desc->loop;
}

std::unique_ptr<AudioService> AudioService::Create(
	const SoundAssetView& sndAssetView,
	std::unique_ptr<IAudioBackend> backend,
	IAssetAsyncLoader* asyncLoader,
	int maxVoices,
	int maxStreams) noexcept
{
	if (backend == nullptr)
		return nullptr;

	std::unique_ptr<AudioService> service(new AudioService(std::move(backend), sndAssetView, asyncLoader));
	if (!service->Initialize(maxVoices, maxStreams))
		return nullptr;

	return service;
}

SoundHandle AudioService::AcquireStaticSound(std::string_view soundID)
{
	auto staticSoundTable = m_sndAssetView.staticSoundTable;
	if (staticSoundTable == nullptr)
		return SoundHandle::Invalid();

	auto desc = staticSoundTable->GetDescriptor(soundID);
	if (desc == nullptr)
		return SoundHandle::Invalid();

	return m_repository.AcquireStaticSound(desc);
}

SoundHandle AudioService::AcquireStreamSound(std::string_view soundID)
{
	auto streamSoundTable = m_sndAssetView.streamSoundTable;
	if (streamSoundTable == nullptr)
		return SoundHandle::Invalid();

	auto desc = streamSoundTable->GetDescriptor(soundID);
	if (desc == nullptr)
		return SoundHandle::Invalid();

	return m_repository.AcquireStreamSound(desc);
}

VoiceHandle AudioService::Play(SoundHandle sh) noexcept
{
	auto loaded = m_repository.Find(sh);
	if (!loaded)
		return VoiceHandle::Invalid();

	if (loaded->state != SoundLoadState::Ready)
	{
		auto desc = loaded->desc;
		if (desc->sndType == SoundType::Stream)
			return EnqueueDeferred(sh, desc);

		return VoiceHandle::Invalid();
	}

	return m_voicePool.Play(sh, loaded, GetParams(loaded->desc));
}

bool AudioService::Pause(VoiceHandle vh) noexcept
{
	return m_voicePool.Pause(vh);
}

bool AudioService::Resume(VoiceHandle vh) noexcept
{
	return m_voicePool.Resume(vh);
}

bool AudioService::Stop(VoiceHandle vh) noexcept
{
	return m_voicePool.StopVoice(vh);
}

bool AudioService::AllStop() noexcept
{
	bool staticOk = m_voicePool.StopVoices(SoundType::Static);
	bool streamOk = m_voicePool.StopVoices(SoundType::Stream);

	return staticOk && streamOk;
}

void AudioService::Update() noexcept
{
	FlushPending();
	m_repository.Update();
	m_voicePool.UpdateVoices();
}

bool AudioService::Unload(SoundHandle sh) noexcept
{
	auto loaded = m_repository.Find(sh);
	if (loaded == nullptr)
		return false;

	if (!m_voicePool.StopVoices(sh)) return false;
	return m_repository.Remove(sh);
}

PlaybackState AudioService::GetState(VoiceHandle vh) const noexcept
{
	return m_voicePool.GetState(vh);
}

void AudioService::SetMasterVolume(float volume) noexcept
{
	m_masterVolume = volume;
}

float AudioService::GetMasterVolume() const noexcept
{
	return m_masterVolume;
}

bool AudioService::SetVolume(VoiceHandle vh, float volume) noexcept
{
	auto desc = m_voicePool.GetDesc(vh);
	if (!desc)
		return false;

	auto group = desc->group;
	if (group == Core::InvalidEnum<AudioGroup>)
		return false;

	float curVolume = GetInstanceVolume(group, volume);
	return m_voicePool.SetVolume(vh, curVolume);
}

AudioService::AudioService(
	std::unique_ptr<IAudioBackend> audioBackend,
	const SoundAssetView& sndAssetView,
	IAssetAsyncLoader* asyncLoader) noexcept
	: m_audioBackend{ std::move(audioBackend) }
	, m_sndAssetView{ sndAssetView }
	, m_repository{ m_audioBackend.get(), asyncLoader }
	, m_voicePool{ m_audioBackend.get() }
{}

bool AudioService::Initialize(int maxVoices, int maxStreams) noexcept
{
	if (!m_audioBackend->Initialize(maxVoices, maxStreams)) return false;
	if (!m_voicePool.Setup(maxVoices, maxStreams)) return false;
	CreateAudioGroup();

	return true;
}

void AudioService::CreateAudioGroup() noexcept
{
	for (auto id : Core::EnumValues<AudioGroup>)
		m_groupInfos.try_emplace(id);
}

PlaybackParams AudioService::GetParams(const SoundDesc* desc) noexcept
{
	PlaybackParams params;
	params.volume = GetInstanceVolume(desc->group, desc->volume);

	switch (desc->sndType)
	{
	case SoundType::Static:
		ApplyStaticParams(static_cast<const StaticSoundDesc*>(desc), params);
		break;
	case SoundType::Stream:
		ApplyStreamParams(static_cast<const StreamSoundDesc*>(desc), params);
		break;
	}

	return params;
}

VoiceHandle AudioService::EnqueueDeferred(SoundHandle sh, const SoundDesc* desc)
{
	auto vh = m_voicePool.AcquireVoiceHandle(desc);
	if (!vh)
		return vh;

	m_pendingPlay.push_back({ vh, sh });
	return vh;
}

void AudioService::FlushPending()
{
	std::erase_if(m_pendingPlay, [this](auto& p) {
		auto loaded = m_repository.Find(p.sound);
		if (!loaded)
			return true;

		if (loaded->state == SoundLoadState::Pending)
			return false;

		if (loaded->state == SoundLoadState::Failed)
		{
			m_voicePool.StopVoice(p.voice);
			return true;
		}

		m_voicePool.Play(p.voice, p.sound, loaded, GetParams(loaded->desc));
		return true;
		});
}

float AudioService::GetGroupVolume(AudioGroup group) const noexcept
{
	float groupVolume = m_groupInfos.at(group).volume;
	float volume = m_masterVolume * groupVolume;

	return std::clamp(volume, 0.0f, 1.0f);
}

float AudioService::GetInstanceVolume(AudioGroup group, float volume) const noexcept
{
	return GetGroupVolume(group) * volume;
}