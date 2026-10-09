export module Service.Audio:AudioService;

import std;
import :VoiceHandle;
import :SoundHandle;
import :LoadedSound;
import :SoundRepository;
import :VoicePool;
import Core.Utils;
import Contract.Asset.Data;
import Contract.Audio.Interfaces;
import Service.IAssetAsyncLoader;

struct GroupInfo
{
	float volume{ 1.0f };
};

struct PendingSoundPlay
{
	VoiceHandle voice;
	SoundHandle sound;
};

static void ApplyStaticParams(const StaticSoundDesc* desc, PlaybackParams& params) noexcept
{
	// Static 전용 파라미터 적용 지점
}

static void ApplyStreamParams(const StreamSoundDesc* desc, PlaybackParams& params) noexcept
{
	params.loop = desc->loop;
}

export class AudioService
{
public:
	~AudioService() = default;
	AudioService() = delete;

	static std::unique_ptr<AudioService> Create(
		const SoundAssetView& sndAssetView,
		std::unique_ptr<IAudioBackend> backend,
		IAssetAsyncLoader* asyncLoader,
		int maxVoices,
		int maxStreams) noexcept
	{
		if (backend == nullptr)
			return nullptr;

		std::unique_ptr<AudioService> service(new AudioService(sndAssetView, std::move(backend), asyncLoader));
		if (!service->Initialize(maxVoices, maxStreams))
			return nullptr;

		return service;
	}

	SoundHandle AcquireStaticSound(std::string_view soundID)
	{
		auto staticSoundTable = m_sndAssetView->staticSoundTable;
		if (staticSoundTable == nullptr)
			return SoundHandle::Invalid();

		auto desc = staticSoundTable->GetDescriptor(soundID);
		if (desc == nullptr)
			return SoundHandle::Invalid();

		return m_repository->AcquireStaticSound(desc);
	}

	SoundHandle AcquireStreamSound(std::string_view soundID)
	{
		auto streamSoundTable = m_sndAssetView->streamSoundTable;
		if (streamSoundTable == nullptr)
			return SoundHandle::Invalid();

		auto desc = streamSoundTable->GetDescriptor(soundID);
		if (desc == nullptr)
			return SoundHandle::Invalid();

		return m_repository->AcquireStreamSound(desc);
	}

	VoiceHandle Play(SoundHandle sh) noexcept
	{
		auto loaded = m_repository->Find(sh);
		if (!loaded)
			return VoiceHandle::Invalid();

		if (loaded->state != SoundLoadState::Ready)
		{
			auto desc = loaded->desc;
			if (desc->sndType == SoundType::Stream)
				return EnqueueDeferred(sh, desc);

			return VoiceHandle::Invalid();
		}

		return m_voicePool->Play(sh, loaded, GetParams(loaded->desc));
	}

	bool Pause(VoiceHandle vh) noexcept
	{
		return m_voicePool->Pause(vh);
	}

	bool Resume(VoiceHandle vh) noexcept
	{
		return m_voicePool->Resume(vh);
	}

	bool Stop(VoiceHandle vh) noexcept
	{
		return m_voicePool->StopVoice(vh);
	}

	bool AllStop() noexcept
	{
		bool staticOk = m_voicePool->StopVoices(SoundType::Static);
		bool streamOk = m_voicePool->StopVoices(SoundType::Stream);

		return staticOk && streamOk;
	}

	void Update() noexcept
	{
		FlushPending();
		m_repository->Update();
		m_voicePool->UpdateVoices();
	}

	bool Unload(SoundHandle sh) noexcept
	{
		auto loaded = m_repository->Find(sh);
		if (loaded == nullptr)
			return false;

		if (!m_voicePool->StopVoices(sh)) return false;
		return m_repository->Remove(sh);
	}

	PlaybackState GetState(VoiceHandle vh) const noexcept
	{
		return m_voicePool->GetState(vh);
	}

	void SetMasterVolume(float volume) noexcept
	{
		m_masterVolume = volume;
	}

	float GetMasterVolume() const noexcept
	{
		return m_masterVolume;
	}

	bool SetVolume(VoiceHandle vh, float volume) noexcept
	{
		auto desc = m_voicePool->GetDesc(vh);
		if (!desc)
			return false;

		auto group = desc->group;
		if (group == Core::InvalidEnum<AudioGroup>)
			return false;

		float curVolume = GetInstanceVolume(group, volume);
		return m_voicePool->SetVolume(vh, curVolume);
	}

private:
	AudioService(
		const SoundAssetView& sndAssetView,
		std::unique_ptr<IAudioBackend> audioBackend,
		IAssetAsyncLoader* asyncLoader) noexcept
		: m_sndAssetView{ std::make_unique<SoundAssetView>(sndAssetView) }
		, m_audioBackend{ std::move(audioBackend) }
		, m_repository{ std::make_unique<SoundRepository>(m_audioBackend.get(), asyncLoader) }
		, m_voicePool{ std::make_unique<VoicePool>(m_audioBackend.get()) }
	{
	}

	bool Initialize(int maxVoices, int maxStreams) noexcept
	{
		if (!m_audioBackend->Initialize(maxVoices, maxStreams)) return false;
		if (!m_voicePool->Setup(maxVoices, maxStreams)) return false;
		CreateAudioGroup();

		return true;
	}

	void CreateAudioGroup() noexcept
	{
		for (auto id : Core::EnumValues<AudioGroup>)
			m_groupInfos[id] = std::make_unique<GroupInfo>();
	}

	PlaybackParams GetParams(const SoundDesc* desc) noexcept
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

	VoiceHandle EnqueueDeferred(SoundHandle sh, const SoundDesc* desc)
	{
		auto vh = m_voicePool->AcquireVoiceHandle(desc);
		if (!vh)
			return vh;

		m_pendingPlay.push_back({ vh, sh });
		return vh;
	}

	void FlushPending()
	{
		for (auto it = m_pendingPlay.begin(); it != m_pendingPlay.end();)
		{
			auto& p = *it;
			auto loaded = m_repository->Find(p.sound);
			if (!loaded)
			{
				it = m_pendingPlay.erase(it);
				continue;
			}

			if (loaded->state == SoundLoadState::Pending)
			{
				++it;
				continue;
			}

			if (loaded->state == SoundLoadState::Failed)
			{
				m_voicePool->StopVoice(p.voice);
				it = m_pendingPlay.erase(it);
				continue;
			}

			m_voicePool->Play(p.voice, p.sound, loaded, GetParams(loaded->desc));
			it = m_pendingPlay.erase(it);
		}
	}

	float GetGroupVolume(AudioGroup group) const noexcept
	{
		float groupVolume = m_groupInfos.at(group)->volume;
		float volume = m_masterVolume * groupVolume;

		return std::clamp(volume, 0.0f, 1.0f);
	}

	float GetInstanceVolume(AudioGroup group, float volume) const noexcept
	{
		return GetGroupVolume(group) * volume;
	}

private:
	std::unique_ptr<SoundAssetView> m_sndAssetView;
	std::unique_ptr<IAudioBackend> m_audioBackend;
	std::unique_ptr<SoundRepository> m_repository;
	std::unique_ptr<VoicePool> m_voicePool;
	float m_masterVolume{ 1.0f };
	std::unordered_map<AudioGroup, std::unique_ptr<GroupInfo>> m_groupInfos;
	std::vector<PendingSoundPlay> m_pendingPlay;
};