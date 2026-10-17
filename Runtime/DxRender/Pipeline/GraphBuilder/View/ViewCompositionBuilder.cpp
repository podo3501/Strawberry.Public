module Pipeline.GraphBuilder;

import :ViewComposition;
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


ViewTargetResources& ViewCompositionBuilder::AcquireActiveViewTarget(
    const ViewTargetPacket& targetInfo,
    std::bitset<MaxViewCount>& activeViews)
{
    activeViews.set(targetInfo.id);
    auto size = Core::ToSize(targetInfo.viewport.width, targetInfo.viewport.height);
    return m_viewPool.Acquire(targetInfo.id, size);
}

void ViewCompositionBuilder::AppendSceneViewOutputs(
    const FramePacket& framePacket,
    const FramePassContext& frameCtx,
    std::bitset<MaxViewCount>& activeViews,
    std::vector<ViewRenderOutput>& outputs)
{
    for (const auto& view : framePacket.sceneViews)
    {
        ViewTargetResources& target = AcquireActiveViewTarget(view->target, activeViews);
        outputs.push_back(m_sceneViewBuilder.Build(m_graph, frameCtx, target, view));
    }
}

void ViewCompositionBuilder::AppendOverlayViewOutputs(
    const FramePacket& framePacket,
    std::bitset<MaxViewCount>& activeViews,
    std::vector<ViewRenderOutput>& outputs)
{
    for (const auto& view : framePacket.overlayViews)
    {
        ViewTargetResources& target = AcquireActiveViewTarget(view->target, activeViews);
        outputs.push_back(m_overlayViewBuilder.Build(m_graph, target, view));
    }
}