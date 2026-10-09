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

    void Add(std::shared_ptr<IResource> res)
    {
        if (!res)
            return;

        m_pendingReleases.emplace_back(std::move(res));
    }

    void Flush()
    {
        if (m_pendingReleases.empty())
            return;

        m_taskScheduler.DeferRelease(std::move(m_pendingReleases));
    }

private:
    TaskScheduler& m_taskScheduler;
    std::vector<std::shared_ptr<IResource>> m_pendingReleases;
};