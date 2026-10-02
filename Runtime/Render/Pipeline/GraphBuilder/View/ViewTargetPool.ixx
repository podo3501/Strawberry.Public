export module Pipeline.GraphBuilder:ViewTargetPool;

import std;
import Core.Math;
import Client.Render.View;
import Runtime.Render.Release;
import Runtime.Render.Factory;
import Runtime.Render.Resource;
import Runtime.Render.Task;
import Runtime.Render.Core;
import Runtime.Render.RGResourceID;

export class ViewTargetPool
{
public:
    ~ViewTargetPool() = default;
    ViewTargetPool() = delete;

    ViewTargetPool(
        Device& device,
        TaskScheduler& taskScheduler,
        DescriptorFactory& descFactory) noexcept
        : m_device{ device }
        , m_descFactory{ descFactory }
        , m_deferredReleaser{ taskScheduler }
    {
    }

    ViewTargetResource& Acquire(
        ViewID id,
        RGResourceIDAllocator& idAllocator,
        const Core::Size& requiredSize)
    {
        auto& view = m_views[id];
        if (view)
        {
            if (view->GetSize() == requiredSize)
                return *view;

            m_deferredReleaser.Add(view);
            view.reset();
        }

        view = std::make_shared<ViewTargetResource>();
        view->Initialize(m_device, m_descFactory, idAllocator, requiredSize);

        return *view;
    }

    void ApplyResourceBindings(ResourceContext& resCtx) const
    {
        for (const auto& view : m_views)
        {
            if (!view) continue;

            resCtx.Set(view->GetColorID(), view->GetColorResource());
            resCtx.Set(view->GetDepthID(), view->GetDepthResource());
        }
    }

    void PruneUnused(const std::bitset<MaxViewCount>& activeViews)
    {
        for (std::size_t i = 0; i < m_views.size(); ++i)
        {
            auto& view = m_views[i];
            if (!view)
                continue;

            if (activeViews.test(i))
                continue;

            m_deferredReleaser.Add(view);
            view.reset();
        }
    }

    void Update()
    {
        m_deferredReleaser.Flush();
    }

private:
    Device& m_device;
    DescriptorFactory& m_descFactory;
    DeferredReleaser m_deferredReleaser;

    std::array<std::shared_ptr<ViewTargetResource>, MaxViewCount> m_views{};
};