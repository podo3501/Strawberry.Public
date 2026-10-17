export module Pipeline.GraphBuilder:ViewComposition;

import std;
import :SceneView;
import :OverlayView;
import :Composite;
import :ViewTargetPool;
import Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Core;
import DxRender.Task;
import DxRender.Factory;
import DxRender.SwapChainPresenter;
import DxRender.Definition;
import Contract.Render.Types;

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
        RGResourceIDAllocator& idAllocator);

    void Update();
    void ApplyResourceBindings(ResourceContext& resources);

    void Build(
        RGResourceID hBackBuffer,
        const FramePacket& framePacket,
        const FramePassContext& frameCtx);

private:
    ViewTargetResources& AcquireActiveViewTarget(
        const ViewTargetPacket& targetInfo,
        std::bitset<MaxViewCount>& activeViews);

    void AppendSceneViewOutputs(
        const FramePacket& framePacket,
        const FramePassContext& frameCtx,
        std::bitset<MaxViewCount>& activeViews,
        std::vector<ViewRenderOutput>& outputs);

    void AppendOverlayViewOutputs(
        const FramePacket& framePacket,
        std::bitset<MaxViewCount>& activeViews,
        std::vector<ViewRenderOutput>& outputs);

private:
    ViewTargetPool m_viewPool;
    SceneViewGraphBuilder m_sceneViewBuilder;
    OverlayViewGraphBuilder m_overlayViewBuilder;
    CompositeGraphBuilder m_compositeBuilder;

    RenderGraph& m_graph;
    RGResourceIDAllocator& m_idAllocator;
};