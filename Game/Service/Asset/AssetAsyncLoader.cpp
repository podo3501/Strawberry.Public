module Service.Asset;

import :AsyncLoader;
import std;
import Core.Logger;

void AssetAsyncLoader::Shutdown()
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

AssetRequestID AssetAsyncLoader::PushRequest(AssetRequest req)
{
    std::lock_guard<std::mutex> lock(m_pendingMutex);

    AssetRequestID id = m_requests.Emplace(std::move(req));
    m_pending.push(id);

    {
        std::lock_guard<std::mutex> waitLock(m_waitMutex); // 작업자가 결과를 넣기 전에 기록되도록 pendingMutex를 잡은 채로 한다
        m_inFlight.insert(id);
    }

    m_pendingCV.notify_one();

    return id;
}

std::optional<AssetRequest> AssetAsyncLoader::TakeRequest(AssetRequestID id)
{
    std::lock_guard<std::mutex> lock(m_pendingMutex);
    return m_requests.Take(id);
}

void AssetAsyncLoader::PushResult(AssetRequestID id, AssetPtr result)
{
    std::lock_guard<std::mutex> lock(m_waitMutex);
    m_inFlight.erase(id);
    m_results.Insert(id, std::move(result));
    m_waitCV.notify_all();
}

std::optional<AssetPtr> AssetAsyncLoader::TakeResult(AssetRequestID id)
{
    std::lock_guard<std::mutex> lock(m_waitMutex);
    return m_results.Take(id);
}

bool AssetAsyncLoader::WaitPopPending(AssetRequestID& outId)
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

AssetPtr AssetAsyncLoader::Wait(AssetRequestID id)
{
    std::unique_lock lock(m_waitMutex);

    m_waitCV.wait(lock, [&] {
        return m_results.Contains(id) || !m_inFlight.contains(id) || m_shutdown.load();
        });

    if (m_shutdown.load())
        return nullptr;

    auto result = m_results.Take(id);
    if (!result)
    {
        Core::Logger::Error("AssetAsyncLoader: 요청한 적 없거나 결과를 이미 꺼낸 ID를 기다림: {}", id);
        return nullptr;
    }

    return std::move(*result);
}

bool AssetAsyncLoader::HasPendingWork() const
{
    std::scoped_lock lock(m_pendingMutex);

    const bool hasPending = !m_pending.empty();
    const bool hasRequests = m_requests.Size() != 0;

    return hasPending || hasRequests;
}