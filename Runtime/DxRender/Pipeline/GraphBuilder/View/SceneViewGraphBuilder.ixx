export module Pipeline.GraphBuilder:SceneView;

import :ViewTargetClear;
import :Skybox;
import :Opaque;
import :DebugSurface;
import Pipeline.Renderer;
import DxRender.RGResourceID;
import DxRender.Graph;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Resource;
import DxRender.Definition;
import Contract.Render.Definition;

// 프레임 전체에 걸쳐 고정되는 컨텍스트 (뷰 루프 시작 전 한 번만 구성)
export struct FramePassContext
{
    const DirectionalLightData& light;
    const ShadowResource& shadowRes;
    RGResourceID hShadow;
};

export class SceneViewGraphBuilder
{
public:
    SceneViewGraphBuilder(
        Device& device,
        DescriptorFactory& descFactory,
        Renderers& renderers)
        : m_clearBuilder{ descFactory }
        , m_skyboxBuilder{ renderers.GetSkyboxRenderer(), descFactory }
        , m_opaqueBuilder{ renderers.GetSurfRenderer(), descFactory }
        , m_debugBuilder{ renderers.GetDebugSurfRenderer(), descFactory }
    {
    }

    ViewRenderOutput Build(
        RenderGraph& graph,
        const FramePassContext& frameCtx,
        const ViewTargetResource& target,
        const std::shared_ptr<SceneViewPacket>& view)
    {
        graph.ImportResource(target.GetColorID(), RGAccess::SRV);
        graph.ImportResource(target.GetDepthID(), RGAccess::DepthWrite);

        m_clearBuilder.Build(graph, target);

        if (view->environment)
            m_skyboxBuilder.Build(graph, view, target);
        if (!view->surface.empty())
            m_opaqueBuilder.Build(graph, frameCtx.light, frameCtx.shadowRes, frameCtx.hShadow, view, target);
        if (!view->debugSurface.empty())
            m_debugBuilder.Build(graph, view, target);

        graph.ExportResource(target.GetColorID(), RGAccess::SRV);
        graph.ExportResource(target.GetDepthID(), RGAccess::DepthWrite);

        return { view->target.id, view->target.viewport, target.GetHeapIndex(), target.GetColorID() };
    }

private:
    ViewTargetClearGraphBuilder m_clearBuilder;
    SkyboxGraphBuilder m_skyboxBuilder;
    OpaqueGraphBuilder m_opaqueBuilder;
    DebugSurfaceGraphBuilder m_debugBuilder;
};