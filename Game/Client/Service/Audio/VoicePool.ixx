export module Service.Audio:VoicePool;

import std;
import :VoiceHandle;
import :SoundHandle;
import :Voice;
import :LoadedSound;
import Core.Utils;
import Core.Handle;
import Contract.Asset.Data;
import Contract.Audio.Interfaces;

static void SortStealCandidateList(std::vector<Voice*>& stealCandidates)
{
	std::ranges::sort(stealCandidates, [](Voice* lhs, Voice* rhs) {
		if (lhs->desc.priority != rhs->desc.priority)
			return lhs->desc.priority < rhs->desc.priority; // 낮은 priority부터 없앰

		return lhs->playbackTime > rhs->playbackTime ||
			(lhs->playbackTime == rhs->playbackTime && lhs < rhs); // 오래된 재생 먼저 없앰
		});
}

static void RemoveFromStealList(std::vector<Voice*>& voices, Voice* v)
{
	auto it = std::find(voices.begin(), voices.end(), v);
	if (it != voices.end())
	{
		*it = voices.back();
		voices.pop_back();
	}
}

export class VoicePool
{
public:
	VoicePool() = delete;

	explicit VoicePool(IAudioBackend* audioBackend)
		: m_audioBackend{ audioBackend }
	{
	}

	~VoicePool() = default;

	bool Setup(int maxVoices, int maxStreams) noexcept
	{
		if (maxVoices > MaxHandles)
			return false;
		if (maxVoices < 0 || maxStreams < 0)
			return false;
		if (maxVoices < maxStreams)
			return false;

		m_voices.Setup(static_cast<std::uint16_t>(maxVoices));

		m_maxVoices = maxVoices;
		m_maxStreams = maxStreams;

		return true;
	}

	VoiceHandle AcquireVoiceHandle(const SoundDesc* desc) noexcept
	{
		const bool isStream = (desc->sndType == SoundType::Stream);
		auto& stealList = isStream ? m_stealStreams : m_stealStatics;
		const std::size_t limit = isStream ? static_cast<std::size_t>(m_maxStreams) : static_cast<std::size_t>(m_maxVoices);

		VoiceHandle vh = (stealList.size() >= limit)
			? StealAndAcquire(desc, stealList)
			: m_voices.Emplace();

		if (!vh)
			return VoiceHandle::Invalid();

		return vh;
	}

	VoiceHandle Play(SoundHandle sh, const LoadedSound* loaded, const PlaybackParams& params) noexcept
	{
		auto desc = loaded->desc;
		auto vh = AcquireVoiceHandle(desc);
		if (!vh)
			return vh;

		if (!Play(vh, sh, loaded, params))
			return VoiceHandle::Invalid();

		return vh;
	}

	bool Play(VoiceHandle vh, SoundHandle sh, const LoadedSound* loaded, const PlaybackParams& params) noexcept
	{
		auto instance = CreateInstance(loaded);
		if (!instance)
			return false;

		if (!instance->Reset(params))
			return false;

		if (!instance->Play())
			return false;

		ActivateVoice(vh, sh, instance, loaded->desc);

		return true;
	}

	bool Pause(VoiceHandle vh) noexcept
	{
		auto instance = GetInstance(vh);
		if (instance == nullptr)
			return false;

		return instance->Pause();
	}

	bool Resume(VoiceHandle vh) noexcept
	{
		auto instance = GetInstance(vh);
		if (instance == nullptr)
			return false;

		return instance->Resume();
	}

	bool StopVoice(VoiceHandle vh) noexcept
	{
		auto voice = m_voices.Get(vh);
		if (!voice)
			return false;

		if (voice->desc.sndType == SoundType::Stream)
			RemoveFromStealList(m_stealStreams, voice);

		if (voice->desc.sndType == SoundType::Static)
			RemoveFromStealList(m_stealStatics, voice);

		if (!voice->StopAndReset()) return false;

		m_voices.Remove(vh);
		return true;
	}

	bool StopVoices(SoundHandle sh) noexcept
	{
		std::vector<VoiceHandle> toStop;

		m_voices.Visit([&toStop, sh](VoiceHandle h, Voice& voice) {
			if (!voice.instance)
				return;

			if (voice.soundHandle == sh)
				toStop.push_back(h);
			});

		for (auto& h : toStop)
			if (!StopVoice(h)) return false;

		return true;
	}

	bool StopVoices(SoundType type) noexcept
	{
		std::vector<VoiceHandle> toStop;

		m_voices.Visit([&toStop, type](VoiceHandle h, Voice& voice) {
			if (!voice.instance)
				return;

			if (voice.desc.sndType == type)
				toStop.push_back(h);
			});

		for (auto& h : toStop)
			if (!StopVoice(h)) return false;

		return true;
	}

	bool SetVolume(VoiceHandle vh, float volume) noexcept
	{
		auto voice = m_voices.Get(vh);
		if (!voice)
			return false;

		auto instance = voice->instance;
		if (!instance)
			return false;

		return instance->SetVolume(volume);
	}

	PlaybackState GetState(VoiceHandle vh) const noexcept
	{
		auto voice = m_voices.Get(vh);
		if (!voice)
			return Core::InvalidEnum<PlaybackState>;

		if (!voice->instance)
			return PlaybackState::Pending; //sh(sound handle)는 있지만 아직 데이터가 로드되지 않아서 instance가 생성되지 않았다.

		return voice->instance->GetState();
	}

	void UpdateVoices() noexcept
	{
		std::vector<VoiceHandle> toStop; //삭제할 것을 넣어놓는 이유는 visit안에서 삭제하게 되면 iterator가 꼬여서 깨지기 때문이다.

		m_voices.Visit([&](VoiceHandle h, Voice& voice) {
			if (!voice.instance)
				return;

			voice.instance->Update();

			auto state = voice.instance->GetState();
			if (state == PlaybackState::Playing)
				voice.playbackTime += 1;
			else if (state == PlaybackState::Stopped)
				toStop.push_back(h);
			});

		for (auto& h : toStop)
			StopVoice(h);

		SortStealCandidateList(m_stealStreams);
		SortStealCandidateList(m_stealStatics);
	}

	const SoundDesc* GetDesc(VoiceHandle vh) const noexcept
	{
		auto voice = m_voices.Get(vh);
		if (!voice)
			return nullptr;

		return &voice->desc;
	}

private:
	ISoundInstance* CreateInstance(const LoadedSound* loaded)
	{
		auto type = loaded->desc->sndType;
		auto buffer = loaded->buffer.get();
		switch (type)
		{
		case SoundType::Static:
			return m_audioBackend->RequestStaticInstance(buffer);
		case SoundType::Stream:
			return m_audioBackend->RequestStreamInstance(buffer);
		}

		return nullptr;
	}

	VoiceHandle StealAndAcquire(const SoundDesc* desc, std::vector<Voice*>& stealList) noexcept
	{
		for (auto* voice : stealList)
		{
			if (!voice->instance)
				continue;

			if (voice->desc.priority >= desc->priority)
				continue;

			StopVoice(voice->voiceHandle);

			auto vh = m_voices.Emplace();
			if (vh)
				return vh;
		}

		return VoiceHandle::Invalid();
	}

	void ActivateVoice(VoiceHandle vh, SoundHandle sh, ISoundInstance* instance, const SoundDesc* desc) noexcept
	{
		auto* voice = m_voices.Get(vh);
		if (!voice)
			return;

		const bool isStream = (desc->sndType == SoundType::Stream);
		auto& stealList = isStream ? m_stealStreams : m_stealStatics;

		voice->voiceHandle = vh;
		voice->soundHandle = sh;
		voice->instance = instance;
		voice->desc = *desc;
		voice->playbackTime = 0;

		stealList.emplace_back(voice);
	}

	const ISoundInstance* GetInstance(VoiceHandle vh) const noexcept
	{
		auto voice = m_voices.Get(vh);
		return voice ? voice->instance : nullptr;
	}

	ISoundInstance* GetInstance(VoiceHandle vh) noexcept
	{
		return const_cast<ISoundInstance*>(static_cast<const VoicePool*>(this)->GetInstance(vh));
	}

private:
	IAudioBackend* m_audioBackend{ nullptr };
	int m_maxVoices{ 0 };
	int m_maxStreams{ 0 };
	static constexpr int MaxHandles{ 64 };

	Core::FixedHandlePool<Voice, VoiceTag, MaxHandles> m_voices;
	std::vector<Voice*> m_stealStatics;
	std::vector<Voice*> m_stealStreams;
};