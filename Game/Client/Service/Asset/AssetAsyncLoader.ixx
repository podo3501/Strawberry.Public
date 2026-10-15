export module Service.Asset:AsyncLoader;

import std;
import Service.IAssetAsyncLoader;
import :AsyncStore;

export class AssetAsyncLoader : public IAssetAsyncLoader
{
public:
    AssetAsyncLoader() = default;
    ~AssetAsyncLoader() override = default;
    void Shutdown();

    AssetRequestID PushRequest(AssetRequest req);
    std::optional<AssetRequest> TakeRequest(AssetRequestID id);

    void PushResult(AssetRequestID id, AssetPtr result);
    AssetPtr TakeResult(AssetRequestID id);

    bool WaitPopPending(AssetRequestID& outId);
    AssetPtr Wait(AssetRequestID id);

    bool HasPendingWork() const;

private:
    std::atomic<bool> m_shutdown{ false };

    // Pending 그룹 (pendingMutex로 일괄 보호)
    mutable std::mutex m_pendingMutex;
    AssetAsyncStore<AssetRequest> m_requests;
    std::queue<AssetRequestID> m_pending;
    std::condition_variable m_pendingCV;

    // Wait/Result 그룹 (waitMutex로 일괄 보호)
    mutable std::mutex m_waitMutex;
    AssetAsyncStore<AssetPtr> m_results;
    std::condition_variable m_waitCV;
};
