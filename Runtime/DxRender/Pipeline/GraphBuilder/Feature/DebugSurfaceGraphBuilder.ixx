export module Pipeline.GraphBuilder:DebugSurface;

import std;
import :ViewTargetResources;
import DxRender.Graph;
import DxRender.Factory;
import Pipeline.Renderer;
import DxRender.Definition;

export class DebugSurfaceGraphBuilder
{
public:
    ~DebugSurfaceGraphBuilder() = default;
    DebugSurfaceGraphBuilder() = delete;

    DebugSurfaceGraphBuilder(
        DebugSurfaceRenderer& debugSurfRenderer,
        DescriptorFactory& descFactory);

    void Build(
        RenderGraph& graph,
        std::shared_ptr<SceneViewPacket> packet,
        const ViewTargetResources& target);

private:
    DebugSurfaceRenderer& m_debugSurfRenderer;
    DescriptorFactory& m_descFactory;
};