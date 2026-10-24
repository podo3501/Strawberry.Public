module DxRender.Pipeline.GraphBuilder;

import :RenderRecordPool;
import Core.Assert;

RenderRecordPool::~RenderRecordPool()
{
    for (auto& w : m_workers)
        w.thread.request_stop();

    for (auto& w : m_workers)
        w.wake.release(); // stop_token만으로는 세마포어 대기 중인 워커가 깨어나지 않음
}

RenderRecordPool::RenderRecordPool(std::size_t numThreads)
    : m_jobs(numThreads)
    , m_workers(numThreads)
{
    for (std::size_t i = 0; i < numThreads; ++i)
    {
        m_workers[i].thread = std::jthread(
            [this, i](std::stop_token st) { WorkerLoop(st, i); });
    }
}

void RenderRecordPool::ExecuteParallel(std::size_t count, const std::function<void(std::size_t)>& jobFn)
{
    Core::Assert(count <= m_workers.size());
    if (count == 0) return;

    std::latch frameLatch(count);
    m_frameLatch = &frameLatch;

    for (std::size_t i = 0; i < count; ++i)
    {
        m_jobs[i] = [&jobFn, i] { jobFn(i); };
        m_workers[i].wake.release();
    }

    frameLatch.wait();
    m_frameLatch = nullptr;
}

void RenderRecordPool::WorkerLoop(std::stop_token stopToken, std::size_t idx)
{
    while (!stopToken.stop_requested())
    {
        m_workers[idx].wake.acquire();
        if (stopToken.stop_requested())
            break;

        if (m_jobs[idx])
        {
            m_jobs[idx]();
            m_jobs[idx] = nullptr;
        }

        if (m_frameLatch)
            m_frameLatch->count_down();
    }
}