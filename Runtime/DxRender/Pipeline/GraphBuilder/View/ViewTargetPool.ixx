export module Pipeline.GraphBuilder:ViewTargetPool;

import std;
import :ViewTargetResources;
import DxRender.Release;
import DxRender.Factory;
import DxRender.Resource;
import DxRender.Task;
import DxRender.Core;
import DxRender.RGResourceID;
import Core.Math;
import Contract.Render.View;

export class ViewTargetPool
{
public:
    ~ViewTargetPool() = default;
    ViewTargetPool() = delete;

    ViewTargetPool(
        Device& device,
        TaskScheduler& taskScheduler,
        RGResourceIDAllocator& idAllocator,
        DescriptorFactory& descFactory) noexcept;

    ViewTargetResources& Acquire(ViewID id, const Core::Size& requiredSize);
    void ApplyResourceBindings(ResourceContext& resCtx) const;
    void PruneUnused(const std::bitset<MaxViewCount>& activeViews);
    void Update();

private:
    std::shared_ptr<ViewTargetResources> CreateViewTargetResource(const Core::Size& size);
    void ReleaseViews(const ViewTargetResources& res);

    Device& m_device;
    RGResourceIDAllocator& m_idAllocator;
    DescriptorFactory& m_descFactory;
    DeferredReleaser m_deferredReleaser;

    std::array<std::shared_ptr<ViewTargetResources>, MaxViewCount> m_views{};
};