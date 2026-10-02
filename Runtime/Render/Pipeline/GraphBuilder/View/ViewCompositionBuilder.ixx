export module Pipeline.GraphBuilder:ViewComposition;

import std;
import :SceneView;
import :OverlayView;
import :Composite;
import :ViewTargetPool;
import Pipeline.Renderer;
import Core.Math;
import Runtime.Render.RGResourceID;
import Runtime.Render.Graph;
import Runtime.Render.Core;
import Runtime.Render.Task;
import Runtime.Render.Factory;
import Runtime.Render.SwapChainPresenter;
import Runtime.Render.Definition;
import Client.Render.Definition;

export class ViewCompositionBuilder
{
public:
    ViewCompositionBuilder(
        Device& device,
        TaskScheduler& taskScheduler,
        DescriptorFactory& descFactory,
        Renderers& renderers,
        SwapChainPresenter& swapChain,
        RenderGraph& renderGraph,
        RGResourceIDAllocator& idAllocator) :
        m_viewPool{ device, taskScheduler, descFactory },
        m_sceneViewBuilder{ device, descFactory, renderers },
        m_overlayViewBuilder{ descFactory, renderers },
        m_compositeBuilder{ renderers.GetCompositeRenderer(), swapChain },
        m_graph{ renderGraph },
        m_idAllocator{ idAllocator }
    {}

    void Update()
    {
        m_viewPool.Update();
    }

    void ApplyResourceBindings(ResourceContext& resources)
    {
        m_viewPool.ApplyResourceBindings(resources);
    }

    void Build(
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

private:
    ViewTargetResource& AcquireActiveViewTarget(
        const ViewTargetPacket& targetInfo,
        std::bitset<MaxViewCount>& activeViews)
    {
        activeViews.set(targetInfo.id);
        auto size = Core::ToSize(targetInfo.viewport.width, targetInfo.viewport.height);
        return m_viewPool.Acquire(targetInfo.id, m_idAllocator, size);
    }

    void AppendSceneViewOutputs(
        const FramePacket& framePacket,
        const FramePassContext& frameCtx,
        std::bitset<MaxViewCount>& activeViews,
        std::vector<ViewRenderOutput>& outputs)
    {
        for (const auto& view : framePacket.sceneViews)
        {
            ViewTargetResource& target = AcquireActiveViewTarget(view->target, activeViews);
            outputs.push_back(m_sceneViewBuilder.Build(m_graph, frameCtx, target, view));
        }
    }

    void AppendOverlayViewOutputs(
        const FramePacket& framePacket,
        std::bitset<MaxViewCount>& activeViews,
        std::vector<ViewRenderOutput>& outputs)
    {
        for (const auto& view : framePacket.overlayViews)
        {
            ViewTargetResource& target = AcquireActiveViewTarget(view->target, activeViews);
            outputs.push_back(m_overlayViewBuilder.Build(m_graph, target, view));
        }
    }

private:
    ViewTargetPool m_viewPool;
    SceneViewGraphBuilder m_sceneViewBuilder;
    OverlayViewGraphBuilder m_overlayViewBuilder;
    CompositeGraphBuilder m_compositeBuilder;

    RenderGraph& m_graph;
    RGResourceIDAllocator& m_idAllocator;
};