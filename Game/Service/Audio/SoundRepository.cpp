module Service.Audio;

import :SoundRepository;
import Core.Type;
import Core.Logger;
import Contract.Audio.Assets;
import Service.AssetAsyncHelper;

SoundRepository::SoundRepository(IAudioBackend * audioBackend, IAssetAsyncLoader * asyncLoader)
	: m_audioBackend{ audioBackend }
	, m_asyncLoader{ asyncLoader }
{}

SoundHandle SoundRepository::AcquireStaticSound(const StaticSoundDesc* desc)
{
	return Acquire(desc);
}

SoundHandle SoundRepository::AcquireStreamSound(const StreamSoundDesc* desc)
{
	return Acquire(desc);
}

// 같은 파일의 소리는 버퍼를 나눠 쓰고 읽기 요청도 한 번만 보낸다. 이미 읽힌 버퍼면 바로 Ready로 시작한다.
SoundHandle SoundRepository::Acquire(const SoundDesc* desc)
{
	auto& shared = BuffersOf(desc->sndType)[desc->resID];
	auto buffer = shared.buffer.lock();

	if (!buffer || shared.state == SoundLoadState::Failed) // 읽기에 실패한 버퍼는 다시 쓰지 않고 새로 읽어 본다.
	{
		buffer = CreateBuffer(desc->sndType);
		if (!buffer)
			return SoundHandle::Invalid();

		const auto assetType = (desc->sndType == SoundType::Static)
			? Core::GetTypeID<StaticSoundAsset>()
			: Core::GetTypeID<StreamSoundAsset>();
		const auto requestId = PushRequest(m_asyncLoader, assetType, desc->resID);

		shared = { buffer, SoundLoadState::Pending };
		m_pending.push_back({ desc->sndType, desc->resID, buffer, requestId });
	}

	return m_loadedSounds.Emplace(desc, buffer, shared.state);
}

std::shared_ptr<ISoundBuffer> SoundRepository::CreateBuffer(SoundType type)
{
	return (type == SoundType::Static)
		? m_audioBackend->CreateStaticSoundBuffer()
		: m_audioBackend->CreateStreamSoundBuffer();
}

SoundRepository::SharedBufferMap& SoundRepository::BuffersOf(SoundType type) noexcept
{
	return (type == SoundType::Static) ? m_staticBuffers : m_streamBuffers;
}

void SoundRepository::Update()
{
	for (auto it = m_pending.begin(); it != m_pending.end();)
	{
		auto result = m_asyncLoader->TakeResult(it->requestId);
		if (!result) // 아직 끝나지 않았다. (값이 nullptr이면 로딩 실패다)
		{
			++it;
			continue;
		}

		if (auto buffer = it->buffer.lock()) // 쓰는 소리가 모두 지워졌으면 결과만 받아서 버린다.
			ApplyLoadResult(*it, buffer, std::move(*result));

		it = m_pending.erase(it);
	}
}

void SoundRepository::ApplyLoadResult(const PendingSoundRequest& request, const std::shared_ptr<ISoundBuffer>& buffer, AssetPtr asset)
{
	if (!asset)
		Core::Logger::Error("SoundRepository: 소리 에셋을 읽지 못함 (요청 {})", request.requestId);

	const auto state = (asset && buffer->LoadFromAsset(std::move(asset))) ? SoundLoadState::Ready : SoundLoadState::Failed;

	auto& buffers = BuffersOf(request.type);
	if (auto it = buffers.find(request.resID); it != buffers.end() && it->second.buffer.lock() == buffer)
		it->second.state = state;

	m_loadedSounds.Visit([&](SoundHandle, LoadedSound& sound) { // 이 버퍼를 기다리던 소리가 모두 함께 바뀐다.
		if (sound.buffer == buffer && sound.state == SoundLoadState::Pending)
			sound.state = state;
		});
}

const LoadedSound* SoundRepository::Find(SoundHandle h) const noexcept
{
	return m_loadedSounds.Find(h);
}

bool SoundRepository::Remove(SoundHandle h) noexcept
{
	return m_loadedSounds.Remove(h);
}
