export module Client.Audio:SoundRepository;

import std;
import :SoundHandle;
import :VoiceHandle;
import :LoadedSound;
import Core.ResourceID;
import Core.Handle;
import Client.Asset.Data;
import Client.Audio.Interfaces;
import Client.IAssetAsyncLoader;
import Client.AssetAsyncHelper;

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
	SoundRepository() = delete;

	SoundRepository(IAudioBackend* audioBackend, IAssetAsyncLoader* asyncLoader)
		: m_audioBackend{ audioBackend }
		, m_asyncLoader{ asyncLoader }
	{
	}

	~SoundRepository() = default;

	SoundHandle AcquireStaticSound(const StaticSoundDesc* desc)
	{
		std::shared_ptr<ISoundBuffer> sndBuffer;

		auto it = m_buffers.find(desc->resID);
		if (it != m_buffers.end())
		{
			sndBuffer = it->second.lock();
		}

		if (!sndBuffer)
		{
			sndBuffer = m_audioBackend->CreateStaticSoundBuffer();
			if (!sndBuffer)
				return SoundHandle::Invalid();
		}

		auto handle = m_loadedSounds.Emplace(desc, sndBuffer, SoundLoadState::Pending);
		auto reqID = PushRequest<StaticSoundAsset>(m_asyncLoader, desc->resID);
		m_pending.push_back({ handle, reqID });

		return handle;
	}

	SoundHandle AcquireStreamSound(const StreamSoundDesc* desc)
	{
		std::shared_ptr<ISoundBuffer> sndBuffer;

		auto it = m_buffers.find(desc->resID);
		if (it != m_buffers.end())
		{
			sndBuffer = it->second.lock();
		}

		if (!sndBuffer)
		{
			sndBuffer = m_audioBackend->CreateStreamSoundBuffer();
			if (!sndBuffer)
				return SoundHandle::Invalid();
		}

		auto handle = m_loadedSounds.Emplace(desc, sndBuffer, SoundLoadState::Pending);
		auto reqID = PushRequest<StreamSoundAsset>(m_asyncLoader, desc->resID);
		m_pending.push_back({ handle, reqID });

		return handle;
	}

	void Update()
	{
		if (m_pending.empty())
			return;

		for (auto it = m_pending.begin(); it != m_pending.end();)
		{
			auto& req = *it;
			auto asset = m_asyncLoader->TakeResult(req.requestId);
			if (!asset)
			{
				++it;
				continue;
			}

			auto sound = m_loadedSounds.Find(req.handle);
			if (!sound) // 이미 unload 되었거나 제거된 경우
			{
				it = m_pending.erase(it);
				continue;
			}

			auto& buffer = sound->buffer;
			bool result = buffer->LoadFromAsset(asset);
			sound->state = result ? SoundLoadState::Ready : SoundLoadState::Failed;

			it = m_pending.erase(it);
		}
	}

	const LoadedSound* Find(SoundHandle h) const noexcept
	{
		return m_loadedSounds.Find(h);
	}

	bool Remove(SoundHandle h) noexcept
	{
		for (auto it = m_pending.begin(); it != m_pending.end();)
		{
			if (it->handle == h)
				it = m_pending.erase(it);
			else
				++it;
		}

		return m_loadedSounds.Remove(h);
	}

private:
	IAudioBackend* m_audioBackend{ nullptr };
	IAssetAsyncLoader* m_asyncLoader{ nullptr };
	std::unordered_map<Core::ResourceID, std::weak_ptr<ISoundBuffer>> m_buffers;
	Core::HandlePool<LoadedSound, SoundTag> m_loadedSounds;

	std::vector<PendingSoundRequest> m_pending;
};