export module DxRender.Pipeline.GraphBuilder:SceneView;

import :ViewTypes;
import :ViewTargetClear;
import :Skybox;
import :Opaque;
import :DebugSurface;
import DxRender.Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Resource;
import Contract.Render.Backend.Types;

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
        Renderers& renderers);

    ViewRenderOutput Build(
        RenderGraph& graph,
        const FramePassContext& frameCtx,
        const ViewTargetResources& target,
        const std::shared_ptr<SceneViewPacket>& view);

private:
    ViewTargetClearGraphBuilder m_clearBuilder;
    SkyboxGraphBuilder m_skyboxBuilder;
    OpaqueGraphBuilder m_opaqueBuilder;
    DebugSurfaceGraphBuilder m_debugBuilder;
};