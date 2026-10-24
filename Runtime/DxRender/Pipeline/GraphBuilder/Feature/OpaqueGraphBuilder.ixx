export module DxRender.Pipeline.GraphBuilder:Opaque;

import std;
import :RenderRecordPool;
import :ViewTargetResources;
import DxRender.Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Resource;
import DxRender.Definition;
import Contract.Render.Shared.View;

export class OpaqueGraphBuilder
{
public:
    ~OpaqueGraphBuilder() = default;
    OpaqueGraphBuilder() = delete;
    OpaqueGraphBuilder(SurfaceRenderer& surfRenderer, DescriptorFactory& descFactory);

    void Build(
        RenderGraph& graph,
        const DirectionalLightData& light,
        const ShadowResource& shadowRes,
        RGResourceID shadowResID,
        std::shared_ptr<SceneViewPacket> packet,
        const ViewTargetResources& target);

private:
    SurfaceRenderer& m_surfRenderer;
    DescriptorFactory& m_descFactory;
    RenderRecordPool m_recordPool;
};