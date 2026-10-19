export module Pipeline.GraphBuilder:RenderRecordPool;

import std;

export class RenderRecordPool
{
public:
    ~RenderRecordPool();
    explicit RenderRecordPool(std::size_t numThreads);
    RenderRecordPool(const RenderRecordPool&) = delete;
    RenderRecordPool& operator=(const RenderRecordPool&) = delete;

    void ExecuteParallel(std::size_t count, const std::function<void(std::size_t)>& jobFn);  // count개의 작업을 워커에 분배해서 병렬 실행하고, 전부 끝날 때까지 블로킹.  jobFn(i)는 워커 i가 실행. count는 반드시 <= WorkerCount().
    std::size_t WorkerCount() const noexcept { return m_workers.size(); }

private:
    struct Worker
    {
        std::jthread thread;
        std::binary_semaphore wake{ 0 };
    };

    void WorkerLoop(std::stop_token stopToken, std::size_t idx);

    std::vector<std::function<void()>> m_jobs; // 메인이 쓰고 워커가 읽음 (semaphore로 happens-before 보장)
    std::latch* m_frameLatch{ nullptr };
    std::vector<Worker> m_workers;
};