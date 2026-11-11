export module Service.Asset:AsyncLoader;

import std;
import Contract.Common.IAssetAsyncLoader;
import :AsyncStore;

export class AssetAsyncLoader : public IAssetAsyncLoader
{
public:
    ~AssetAsyncLoader() override = default;
    AssetAsyncLoader() = default;

    void Shutdown();

    AssetRequestID PushRequest(AssetRequest req) override;
    std::optional<AssetRequest> TakeRequest(AssetRequestID id);

    void PushResult(AssetRequestID id, AssetPtr result);
    std::optional<AssetPtr> TakeResult(AssetRequestID id) override;

    bool WaitPopPending(AssetRequestID& outId);
    AssetPtr Wait(AssetRequestID id) override;

    bool HasPendingWork() const;

private:
    std::atomic<bool> m_shutdown{ false };

    // Pending 그룹 (pendingMutex로 일괄 보호)
    mutable std::mutex m_pendingMutex;
    AssetAsyncStore<AssetRequest> m_requests;
    std::queue<AssetRequestID> m_pending;
    std::condition_variable m_pendingCV;

    // Wait/Result 그룹 (waitMutex로 일괄 보호). 두 mutex를 함께 잡을 때는 pendingMutex를 먼저 잡는다
    mutable std::mutex m_waitMutex;
    AssetAsyncStore<AssetPtr> m_results;
    std::unordered_set<AssetRequestID> m_inFlight; // 요청했고 결과가 아직 오지 않은 ID
    std::condition_variable m_waitCV;
};
