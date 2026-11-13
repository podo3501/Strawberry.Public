export module DxRender.Pipeline.GraphBuilder:ViewTargetPool;

import std;
import :ViewTargetResources;
import DxRender.Release;
import DxRender.Factory;
import DxRender.Resource;
import DxRender.Task;
import DxRender.Core;
import DxRender.RGResourceID;
import Core.Math;
import Contract.Render.Shared.View;

// 뷰 타겟 한 변의 최대 크기 (D3D12의 2D 텍스처 한도와 같다)
export constexpr std::uint32_t MaxViewTargetSide = 16384;

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

    // 뷰 번호가 범위 밖이거나 크기가 올바르지 않거나 만들지 못하면 Error와 nullptr (그 뷰는 그리지 않는다)
    ViewTargetResources* Acquire(ViewID id, const Core::Size& requiredSize);
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