module Service.Audio;

import :LoadedSound;
import Core.Utils;
import Core.Logger;
import Contract.Audio.Assets;

// 그룹이 없거나 모르는 이름인 소리는 쓰지 않는다. 데이터 오류를 소리를 얻는 시점에 알린다.
static bool HasValidGroup(const SoundDesc* desc, std::string_view soundID)
{
	if (desc->group != Core::InvalidEnum<AudioGroup>)
		return true;

	Core::Logger::Error("AudioService: 소리의 그룹이 없거나 알 수 없는 이름임: {}", soundID);
	return false;
}

static void ApplyStaticParams(const StaticSoundDesc* desc, PlaybackParams& params) noexcept
{
	// Static 전용 파라미터 적용 지점
}

static void ApplyStreamParams(const StreamSoundDesc* desc, PlaybackParams& params) noexcept
{
	params.loop = desc->loop;
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

SoundHandle AudioService::AcquireStaticSound(std::string_view soundID)
{
	auto staticSoundTable = m_sndAssetView.staticSoundTable;
	if (staticSoundTable == nullptr)
		return SoundHandle::Invalid();

	auto desc = staticSoundTable->GetDescriptor(soundID);
	if (desc == nullptr || !HasValidGroup(desc, soundID))
		return SoundHandle::Invalid();

	return m_repository.AcquireStaticSound(desc);
}

SoundHandle AudioService::AcquireStreamSound(std::string_view soundID)
{
	auto streamSoundTable = m_sndAssetView.streamSoundTable;
	if (streamSoundTable == nullptr)
		return SoundHandle::Invalid();

	auto desc = streamSoundTable->GetDescriptor(soundID);
	if (desc == nullptr || !HasValidGroup(desc, soundID))
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
	std::erase_if(m_pendingPlay, [vh](const PendingSoundPlay& p) { return p.voice == vh; }); // 대기 재생이면 나중에 시작되지 않게 한다.
	return m_voicePool.StopVoice(vh);
}

bool AudioService::AllStop() noexcept
{
	CancelPendingPlays([](const PendingSoundPlay&) { return true; });

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

	CancelPendingPlays([sh](const PendingSoundPlay& p) { return p.sound == sh; });
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

	// 이미 재생 중인 목소리에도 바로 적용한다.
	m_voicePool.ForEachActiveVoice([this](VoiceHandle vh, const SoundDesc& desc, float baseVolume) {
		if (desc.group == Core::InvalidEnum<AudioGroup>)
			return;

		m_voicePool.SetVolume(vh, GetInstanceVolume(desc.group, baseVolume));
		});
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
	if (!m_voicePool.SetVolume(vh, curVolume))
		return false;

	return m_voicePool.SetBaseVolume(vh, volume); // 마스터 볼륨이 바뀔 때 다시 계산할 수 있게 기록한다.
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

void AudioService::CancelPendingPlays(const std::function<bool(const PendingSoundPlay&)>& shouldCancel)
{
	std::erase_if(m_pendingPlay, [&](const PendingSoundPlay& p) {
		if (!shouldCancel(p))
			return false;

		m_voicePool.StopVoice(p.voice); // 대기하던 목소리 슬롯을 돌려준다.
		return true;
		});
}

void AudioService::FlushPending()
{
	std::erase_if(m_pendingPlay, [this](auto& p) {
		auto loaded = m_repository.Find(p.sound);
		if (!loaded)
		{
			m_voicePool.StopVoice(p.voice);
			return true;
		}

		if (loaded->state == SoundLoadState::Pending)
			return false;

		if (loaded->state == SoundLoadState::Failed)
		{
			m_voicePool.StopVoice(p.voice);
			return true;
		}

		if (!m_voicePool.Play(p.voice, p.sound, loaded, GetParams(loaded->desc)))
			m_voicePool.StopVoice(p.voice); // 재생을 시작하지 못했으니 슬롯을 돌려준다.

		return true;
		});
}

float AudioService::GetGroupVolume(AudioGroup group) const noexcept
{
	const auto it = m_groupInfos.find(group); // 모르는 그룹이어도 예외로 죽지 않게 그룹 볼륨은 1.0으로 본다.
	float groupVolume = (it != m_groupInfos.end()) ? it->second.volume : 1.0f;
	float volume = m_masterVolume * groupVolume;

	return std::clamp(volume, 0.0f, 1.0f);
}

float AudioService::GetInstanceVolume(AudioGroup group, float volume) const noexcept
{
	return GetGroupVolume(group) * volume;
}

std::unique_ptr<IAudioService> CreateAudioService(
	std::unique_ptr<IAudioBackend> backend,
	const SoundAssetView& sndAssetView,
	IAssetAsyncLoader* asyncLoader,
	int maxVoices,
	int maxStreams) noexcept
{
	if (backend == nullptr)
		return nullptr;

	auto service = std::make_unique<AudioService>(std::move(backend), sndAssetView, asyncLoader);
	if (!service->Initialize(maxVoices, maxStreams))
		return nullptr;

	return service;
}