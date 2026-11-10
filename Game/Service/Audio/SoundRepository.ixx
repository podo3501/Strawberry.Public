export module Service.Audio:SoundRepository;

import std;
import :LoadedSound;
import Core.ResourceID;
import Core.Handle;
import Contract.Audio.Assets;
import Contract.Audio.Service;
import Contract.Audio.Backend;
import Service.AssetAsyncHelper;

// 같은 파일의 소리가 함께 쓰는 버퍼. 버퍼는 이 목록이 아니라 소리(LoadedSound)들이 붙들고 있다.
struct SharedBuffer
{
	std::weak_ptr<ISoundBuffer> buffer;
	SoundLoadState state{ SoundLoadState::Pending };
};

// 버퍼 하나를 읽어 달라고 보낸 요청. 그 버퍼를 쓰는 소리가 모두 지워지면 결과가 와도 버린다.
struct PendingSoundRequest
{
	SoundType type;
	Core::ResourceID resID;
	std::weak_ptr<ISoundBuffer> buffer;
	AssetRequestID requestId;
};

export class SoundRepository
{
public:
	~SoundRepository() = default;
	SoundRepository() = delete;
	SoundRepository(IAudioBackend* audioBackend, IAssetAsyncLoader* asyncLoader);

	SoundHandle AcquireStaticSound(const StaticSoundDesc* desc);
	SoundHandle AcquireStreamSound(const StreamSoundDesc* desc);

	void Update();
	bool Remove(SoundHandle h) noexcept;
	const LoadedSound* Find(SoundHandle h) const noexcept;

private:
	using SharedBufferMap = std::unordered_map<Core::ResourceID, SharedBuffer>;

	SoundHandle Acquire(const SoundDesc* desc);
	std::shared_ptr<ISoundBuffer> CreateBuffer(SoundType type);
	void ApplyLoadResult(const PendingSoundRequest& request, const std::shared_ptr<ISoundBuffer>& buffer, AssetPtr asset);
	SharedBufferMap& BuffersOf(SoundType type) noexcept;

	IAudioBackend* m_audioBackend{ nullptr };
	IAssetAsyncLoader* m_asyncLoader{ nullptr };
	SharedBufferMap m_staticBuffers;   // 같은 파일이라도 정적과 스트림은 버퍼가 달라서 따로 둔다
	SharedBufferMap m_streamBuffers;
	Core::HandlePool<LoadedSound, SoundTag> m_loadedSounds;

	std::vector<PendingSoundRequest> m_pending;
};
