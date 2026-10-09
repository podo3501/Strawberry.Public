export module Pipeline.GraphBuilder:RenderRecordPool;

import std;
import Core.Assert;

export class RenderRecordPool
{
public:
    explicit RenderRecordPool(std::size_t numThreads)
    {
        m_jobs.resize(numThreads);
        m_workers.reserve(numThreads);

        // 1단계: Worker 인스턴스들을 먼저 생성 (아직 스레드는 안 만듦)
        for (std::size_t i = 0; i < numThreads; ++i)
            m_workers.push_back(std::make_unique<Worker>());

        // 2단계: 각 Worker의 jthread 시작
        // (1단계와 분리하는 이유: m_workers가 이 시점 이후 재할당되지 않는다는 걸 보장한 뒤에
        //  WorkerLoop 안에서 m_workers[idx] 접근이 안전하도록)
        for (std::size_t i = 0; i < numThreads; ++i)
        {
            m_workers[i]->thread = std::jthread(
                [this, i](std::stop_token st) { WorkerLoop(st, i); });
        }
    }

    ~RenderRecordPool()
    {
        for (auto& w : m_workers)
            w->thread.request_stop();

        for (auto& w : m_workers)
            w->wake.release(); // stop_token만으로는 세마포어 대기 중인 워커가 깨어나지 않음
    }

    RenderRecordPool(const RenderRecordPool&) = delete;
    RenderRecordPool& operator=(const RenderRecordPool&) = delete;

    void ExecuteParallel(std::size_t count, const std::function<void(std::size_t)>& jobFn)  // count개의 작업을 워커에 분배해서 병렬 실행하고, 전부 끝날 때까지 블로킹.  jobFn(i)는 워커 i가 실행. count는 반드시 <= WorkerCount().
    {
        Core::Assert(count <= m_workers.size());
        if (count == 0) return;

        std::latch frameLatch(count);
        m_frameLatch = &frameLatch;

        for (std::size_t i = 0; i < count; ++i)
        {
            m_jobs[i] = [&jobFn, i] { jobFn(i); };
            m_workers[i]->wake.release();
        }

        frameLatch.wait();
        m_frameLatch = nullptr;
    }

    std::size_t WorkerCount() const noexcept { return m_workers.size(); }

private:
    struct Worker
    {
        std::jthread thread;
        std::binary_semaphore wake{ 0 };
    };

    void WorkerLoop(std::stop_token stopToken, std::size_t idx)
    {
        while (!stopToken.stop_requested())
        {
            m_workers[idx]->wake.acquire();
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

    std::vector<std::unique_ptr<Worker>> m_workers;
    std::vector<std::function<void()>> m_jobs; // 메인이 쓰고 워커가 읽음 (semaphore로 happens-before 보장)
    std::latch* m_frameLatch{ nullptr };
};