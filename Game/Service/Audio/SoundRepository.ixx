export module Service.Audio:SoundRepository;

import std;
import :LoadedSound;
import Core.ResourceID;
import Core.Handle;
import Contract.Audio.Assets;
import Contract.Audio.Service;
import Contract.Audio.Backend;
import Service.AssetAsyncHelper;

struct PendingSoundRequest
{
	SoundHandle handle;
	AssetRequestID requestId;

	friend bool operator==(const PendingSoundRequest& lhs, const PendingSoundRequest& rhs) noexcept
	{
		return lhs.handle == rhs.handle && lhs.requestId == rhs.requestId;
	}
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
	IAudioBackend* m_audioBackend{ nullptr };
	IAssetAsyncLoader* m_asyncLoader{ nullptr };
	std::unordered_map<Core::ResourceID, std::weak_ptr<ISoundBuffer>> m_buffers;
	Core::HandlePool<LoadedSound, SoundTag> m_loadedSounds;

	std::vector<PendingSoundRequest> m_pending;
};