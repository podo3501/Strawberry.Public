module Service.Audio;

import std;
import :VoicePool;
import Core.Utils;

VoicePool::VoicePool(IAudioBackend* audioBackend)
	: m_audioBackend{ audioBackend }
{}

bool VoicePool::Setup(int maxVoices, int maxStreams) noexcept
{
	if (maxVoices > MaxHandles)
		return false;
	if (maxVoices < 0 || maxStreams < 0)
		return false;
	if (maxVoices < maxStreams)
		return false;

	if (!m_voices.Setup(static_cast<std::size_t>(maxVoices)))
		return false;

	m_maxVoices = maxVoices;
	m_maxStreams = maxStreams;

	return true;
}

// 얻은 목소리는 재생을 시작하기 전까지 대기(Pending) 상태다. 대기 중인 목소리도 한도에 들어가고 뺏길 수 있다.
VoiceHandle VoicePool::AcquireVoiceHandle(const SoundDesc* desc) noexcept
{
	const int limit = (desc->sndType == SoundType::Stream) ? m_maxStreams : m_maxVoices;

	VoiceHandle vh = (CountVoices(desc->sndType) < limit) ? m_voices.Emplace() : VoiceHandle::Invalid();
	if (!vh) // 한도에 닿았거나 다른 종류가 슬롯을 차지해 풀이 가득 찬 경우. 같은 종류만 뺏는다.
	{
		if (!StopVoice(FindStealVictim(desc)))
			return VoiceHandle::Invalid();

		vh = m_voices.Emplace();
	}

	auto* voice = m_voices.Find(vh);
	if (!voice)
		return VoiceHandle::Invalid();

	voice->voiceHandle = vh;
	voice->desc = desc;
	voice->volume = desc->volume;

	return vh;
}

VoiceHandle VoicePool::Play(SoundHandle sh, const LoadedSound* loaded, const PlaybackParams& params) noexcept
{
	auto vh = AcquireVoiceHandle(loaded->desc);
	if (!vh)
		return vh;

	if (!Play(vh, sh, loaded, params))
	{
		StopVoice(vh); // 재생하지 못했으니 얻어 둔 슬롯을 돌려준다.
		return VoiceHandle::Invalid();
	}

	return vh;
}

bool VoicePool::Play(VoiceHandle vh, SoundHandle sh, const LoadedSound* loaded, const PlaybackParams& params) noexcept
{
	auto* voice = m_voices.Find(vh); // 이미 풀린 핸들이면 인스턴스를 빌리지 않는다. 빌리면 추적되지 않는 재생이 생긴다.
	if (!voice)
		return false;

	auto instance = CreateInstance(loaded);
	if (!instance)
		return false;

	if (!instance->Reset(params) || !instance->Play())
	{
		instance->Stop(); // 빌린 인스턴스를 돌려준다.
		return false;
	}

	voice->soundHandle = sh;
	voice->instance = instance;
	voice->playbackTime = 0;

	return true;
}

bool VoicePool::Pause(VoiceHandle vh) noexcept
{
	auto instance = GetInstance(vh);
	if (instance == nullptr)
		return false;

	return instance->Pause();
}

bool VoicePool::Resume(VoiceHandle vh) noexcept
{
	auto instance = GetInstance(vh);
	if (instance == nullptr)
		return false;

	return instance->Resume();
}

bool VoicePool::StopVoice(VoiceHandle vh) noexcept
{
	auto voice = m_voices.Find(vh);
	if (!voice)
		return false;

	voice->StopAndReset();

	m_voices.Remove(vh);
	return true;
}

bool VoicePool::StopVoices(SoundHandle sh) noexcept
{
	return StopWhere([sh](const Voice& voice) { return voice.soundHandle == sh; });
}

bool VoicePool::StopVoices(SoundType type) noexcept
{
	return StopWhere([type](const Voice& voice) { return voice.desc->sndType == type; });
}

bool VoicePool::SetVolume(VoiceHandle vh, float volume) noexcept
{
	auto instance = GetInstance(vh);
	if (!instance)
		return false;

	return instance->SetVolume(volume);
}

// 마스터 볼륨이 바뀌어도 다시 계산할 수 있게, 소리 자체의 볼륨을 목소리에 기록해 둔다.
bool VoicePool::SetBaseVolume(VoiceHandle vh, float baseVolume) noexcept
{
	auto voice = m_voices.Find(vh);
	if (!voice || !voice->instance)
		return false;

	voice->volume = baseVolume;
	return true;
}

void VoicePool::ForEachActiveVoice(const std::function<void(VoiceHandle, const SoundDesc&, float baseVolume)>& visit) noexcept
{
	m_voices.Visit([&visit](VoiceHandle h, Voice& voice) {
		if (voice.instance) // 대기 중인 목소리는 건너뛴다. 재생을 시작할 때 현재 볼륨이 적용된다.
			visit(h, *voice.desc, voice.volume);
		});
}

PlaybackState VoicePool::GetState(VoiceHandle vh) const noexcept
{
	auto voice = m_voices.Find(vh);
	if (!voice)
		return Core::InvalidEnum<PlaybackState>;

	if (!voice->instance)
		return PlaybackState::Pending; // 소리가 아직 읽히지 않아 재생을 시작하지 못한 목소리다.

	return voice->instance->GetState();
}

void VoicePool::UpdateVoices() noexcept
{
	m_voices.Visit([](VoiceHandle, Voice& voice) {
		if (!voice.instance)
			return;

		voice.instance->Update();
		if (voice.instance->GetState() == PlaybackState::Playing)
			voice.playbackTime += 1;
		});

	StopWhere([](const Voice& voice) { // 끝난 소리의 목소리를 풀고 인스턴스를 돌려준다.
		return voice.instance && voice.instance->GetState() == PlaybackState::Stopped;
		});
}

const SoundDesc* VoicePool::GetDesc(VoiceHandle vh) const noexcept
{
	auto voice = m_voices.Find(vh);
	return voice ? voice->desc : nullptr;
}

ISoundInstance* VoicePool::CreateInstance(const LoadedSound* loaded)
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

int VoicePool::CountVoices(SoundType type) const noexcept
{
	int count = 0;
	m_voices.Visit([&count, type](VoiceHandle, const Voice& voice) {
		if (voice.desc->sndType == type)
			++count;
		});

	return count;
}

// 같은 종류에서 우선순위가 더 낮은 목소리 중 가장 낮은 것, 같으면 가장 오래 재생된 것을 고른다. 없으면 Invalid.
VoiceHandle VoicePool::FindStealVictim(const SoundDesc* desc) const noexcept
{
	const Voice* victim = nullptr;
	m_voices.Visit([&victim, desc](VoiceHandle, const Voice& voice) {
		if (voice.desc->sndType != desc->sndType || voice.desc->priority >= desc->priority)
			return;

		const bool isBetter = !victim
			|| voice.desc->priority < victim->desc->priority
			|| (voice.desc->priority == victim->desc->priority && voice.playbackTime > victim->playbackTime);
		if (isBetter)
			victim = &voice;
		});

	return victim ? victim->voiceHandle : VoiceHandle::Invalid();
}

bool VoicePool::StopWhere(const std::function<bool(const Voice&)>& shouldStop) noexcept
{
	std::vector<VoiceHandle> toStop; // 도는 중에 지우지 않으려고 먼저 모은다.
	m_voices.Visit([&](VoiceHandle h, const Voice& voice) {
		if (shouldStop(voice))
			toStop.push_back(h);
		});

	bool allStopped = true;
	for (auto h : toStop)
		allStopped = StopVoice(h) && allStopped;

	return allStopped;
}

const ISoundInstance* VoicePool::GetInstance(VoiceHandle vh) const noexcept
{
	auto voice = m_voices.Find(vh);
	return voice ? voice->instance : nullptr;
}

ISoundInstance* VoicePool::GetInstance(VoiceHandle vh) noexcept
{
	return const_cast<ISoundInstance*>(static_cast<const VoicePool*>(this)->GetInstance(vh));
}
