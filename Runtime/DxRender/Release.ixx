export module DxRender.Release;

import std;
import Contract.Render.IResource;
import DxRender.Task;

export class DeferredReleaser
{
public:
    ~DeferredReleaser() = default;
    DeferredReleaser() = delete;
    explicit DeferredReleaser(TaskScheduler& taskScheduler) noexcept
        : m_taskScheduler{ taskScheduler }
    {
    }

    void Add(std::shared_ptr<void> object)
    {
        if (!object)
            return;

        m_pendingReleases.emplace_back(std::move(object));
    }

    void Flush()
    {
        if (m_pendingReleases.empty())
            return;

        m_taskScheduler.DeferRelease(std::move(m_pendingReleases));
    }

private:
    TaskScheduler& m_taskScheduler;
    std::vector<std::shared_ptr<void>> m_pendingReleases;
};