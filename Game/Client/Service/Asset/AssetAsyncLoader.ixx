export module Client.Asset.Service:AsyncLoader;

import std;
import Client.IAssetAsyncLoader;
import :AsyncStore;
import Core.Assert;

export class AssetAsyncLoader : public IAssetAsyncLoader
{
public:
    AssetAsyncLoader() = default;
    ~AssetAsyncLoader() override = default;

    AssetRequestID PushRequest(AssetRequest req) override
    {
        std::lock_guard<std::mutex> lock(m_pendingMutex);

        AssetRequestID id = m_requests.Emplace(std::move(req));
        m_pending.push(id);

        m_pendingCV.notify_one();

        return id;
    }

    void Shutdown()
    {
        m_shutdown.store(true);

        {
            std::lock_guard lock(m_pendingMutex);
            m_pendingCV.notify_all();
        }

        {
            std::lock_guard lock(m_waitMutex);
            m_waitCV.notify_all();
        }
    }

    bool WaitPopPending(AssetRequestID& outId)
    {
        std::unique_lock lock(m_pendingMutex);

        m_pendingCV.wait(lock, [&] {
            return m_shutdown.load() || !m_pending.empty();
            });

        if (m_shutdown.load())
            return false;

        outId = m_pending.front();
        m_pending.pop();

        return true;
    }

    std::optional<AssetRequest> TakeRequest(AssetRequestID id)
    {
        std::lock_guard<std::mutex> lock(m_pendingMutex);
        return m_requests.Take(id);
    }

    void PushResult(AssetRequestID id, AssetPtr result)
    {
        std::lock_guard<std::mutex> lock(m_waitMutex);
        m_results.Insert(id, std::move(result));
        m_waitCV.notify_all();
    }

    AssetPtr TakeResult(AssetRequestID id) override
    {
        std::lock_guard<std::mutex> lock(m_waitMutex);
        auto result = m_results.Take(id);
        return result.value_or(nullptr);
    }

    AssetPtr Wait(AssetRequestID id) override
    {
        std::unique_lock lock(m_waitMutex);

        m_waitCV.wait(lock, [&] {
            return m_results.Contains(id) || m_shutdown.load();
            });

        if (m_shutdown.load())
            return nullptr;

        auto result = m_results.Take(id);
        Core::Assert(result.has_value());
        return std::move(*result);
    }

    bool HasPendingWork() const
    {
        std::scoped_lock lock(m_pendingMutex);

        const bool hasPending = !m_pending.empty();
        const bool hasRequests = m_requests.Size() != 0;

        return hasPending || hasRequests;
    }

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
