module DxRender.Pipeline.GraphBuilder;

import :DebugSurface;
import DxRender.Command;
import DxRender.Resource;

DebugSurfaceGraphBuilder::DebugSurfaceGraphBuilder(
    DebugSurfaceRenderer& debugSurfRenderer,
    DescriptorFactory& descFactory)
    : m_debugSurfRenderer{ debugSurfRenderer }
    , m_descFactory{ descFactory }
{}

void DebugSurfaceGraphBuilder::Build(
    RenderGraph& graph,
    std::shared_ptr<SceneViewPacket> packet,
    const ViewTargetResources& target)
{
    auto& grid = graph.AddGraphicsPass("DebugSurface_View" + std::to_string(packet->target.id));
    grid.Write(target.GetColorID(), RGAccess::RTV);
    grid.Read(target.GetDepthID(), RGAccess::DepthRead);

    grid.execute =
        [
            &descFactory = m_descFactory,
            &debugSurfRenderer = m_debugSurfRenderer,
            packet,
            colorRTVIndex = target.GetColorRTVIndex(),
            depthDSVIndex = target.GetDepthDSVIndex()
        ]
    (TaskCommandLists cmds, TaskContext& ctx)
        {
            CommandList& cmd = cmds.Single();

            auto rtv = descFactory.GetRTVHandle(colorRTVIndex);
            auto dsv = descFactory.GetDSVHandle(depthDSVIndex);

            CommandUtils::SetRenderTarget(cmd, rtv, dsv);
            CommandUtils::SetViewRect(cmd, packet->target.localViewport);

            debugSurfRenderer.PrepareDraw(cmd, packet->target.camera);

            for (auto& item : packet->debugSurface)
            {
                auto mesh = static_cast<MeshResource*>(item.mesh.get());
                auto material = static_cast<DebugMaterialResource*>(item.material.get());

                debugSurfRenderer.BindPipeline(cmd, material->GetPipelineState());
                debugSurfRenderer.Draw(cmd, *mesh, item.world);
            }
        };
}