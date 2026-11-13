module DxRender.Pipeline.GraphBuilder;

import :ViewComposition;
import Core.Logger;
import Core.Math;

ViewCompositionBuilder::ViewCompositionBuilder(
    Device& device,
    TaskScheduler& taskScheduler,
    DescriptorFactory& descFactory,
    Renderers& renderers,
    SwapChainPresenter& swapChain,
    RenderGraph& renderGraph,
    RGResourceIDAllocator& idAllocator) :
    m_viewPool{ device, taskScheduler, idAllocator, descFactory },
    m_sceneViewBuilder{ device, descFactory, renderers },
    m_overlayViewBuilder{ descFactory, renderers },
    m_compositeBuilder{ renderers.GetCompositeRenderer(), swapChain },
    m_graph{ renderGraph },
    m_idAllocator{ idAllocator }
{}

void ViewCompositionBuilder::Update()
{
    m_viewPool.Update();
}

void ViewCompositionBuilder::ApplyResourceBindings(ResourceContext& resources)
{
    m_viewPool.ApplyResourceBindings(resources);
}

void ViewCompositionBuilder::Build(
    RGResourceID hBackBuffer,
    const FramePacket& framePacket,
    const FramePassContext& frameCtx)
{
    std::vector<ViewRenderOutput> viewOutputs;
    viewOutputs.reserve(framePacket.sceneViews.size() + framePacket.overlayViews.size());
    std::bitset<MaxViewCount> activeViews;

    AppendSceneViewOutputs(framePacket, frameCtx, activeViews, viewOutputs);
    AppendOverlayViewOutputs(framePacket, activeViews, viewOutputs);

    m_viewPool.PruneUnused(activeViews);
    std::stable_sort(viewOutputs.begin(), viewOutputs.end(),
        [](const ViewRenderOutput& lhs, const ViewRenderOutput& rhs) { return lhs.id < rhs.id; });

    m_compositeBuilder.Build(m_graph, hBackBuffer, viewOutputs);
}


// 번호가 범위 밖이거나 한 프레임에 두 번 오거나 뷰 타겟을 얻지 못하면 Error와 nullptr (그 뷰는 만들지 않는다)
ViewTargetResources* ViewCompositionBuilder::AcquireActiveViewTarget(
    const ViewTargetPacket& targetInfo,
    std::bitset<MaxViewCount>& activeViews)
{
    if (targetInfo.id >= MaxViewCount)
    {
        Core::Logger::Error("ViewCompositionBuilder: 뷰 번호 {}가 범위(0~{})를 벗어나 그리지 않는다", targetInfo.id, MaxViewCount - 1);
        return nullptr;
    }
    if (activeViews.test(targetInfo.id))
    {
        Core::Logger::Error("ViewCompositionBuilder: 뷰 번호 {}가 한 프레임에 두 번 와서 나중 것은 그리지 않는다", targetInfo.id);
        return nullptr;
    }

    auto size = Core::ToSize(targetInfo.viewport.width, targetInfo.viewport.height);
    ViewTargetResources* target = m_viewPool.Acquire(targetInfo.id, size);
    if (target)
        activeViews.set(targetInfo.id); // 얻지 못한 뷰는 쓰는 뷰로 세지 않는다
    return target;
}

void ViewCompositionBuilder::AppendSceneViewOutputs(
    const FramePacket& framePacket,
    const FramePassContext& frameCtx,
    std::bitset<MaxViewCount>& activeViews,
    std::vector<ViewRenderOutput>& outputs)
{
    for (const auto& view : framePacket.sceneViews)
    {
        ViewTargetResources* target = AcquireActiveViewTarget(view->target, activeViews);
        if (!target)
            continue;
        outputs.push_back(m_sceneViewBuilder.Build(m_graph, frameCtx, *target, view));
    }
}

void ViewCompositionBuilder::AppendOverlayViewOutputs(
    const FramePacket& framePacket,
    std::bitset<MaxViewCount>& activeViews,
    std::vector<ViewRenderOutput>& outputs)
{
    for (const auto& view : framePacket.overlayViews)
    {
        ViewTargetResources* target = AcquireActiveViewTarget(view->target, activeViews);
        if (!target)
            continue;
        outputs.push_back(m_overlayViewBuilder.Build(m_graph, *target, view));
    }
}