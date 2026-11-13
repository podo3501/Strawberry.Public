module DxRender.Pipeline.GraphBuilder;

import :DebugSurface;
import DxRender.Command;
import DxRender.Resource;
import Core.Logger;
import Core.TypeHierarchy;

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

            if (!debugSurfRenderer.PrepareDraw(cmd, packet->target.camera))
                return; // 프레임 상수 한도를 넘었다: 이 뷰의 디버그 표면은 그리지 않는다

            for (auto& item : packet->debugSurface)
            {
                auto mesh = Core::Cast<MeshResource>(item.mesh.get());
                auto material = Core::Cast<DebugMaterialResource>(item.material.get());
                if (!mesh || !material)
                {
                    Core::Logger::Error("DebugSurfaceGraphBuilder: 메시나 디버그 재질이 올바른 리소스가 아니라 그리지 않는다");
                    continue;
                }

                if (!debugSurfRenderer.BindPipeline(cmd, material->GetPipelineState()))
                    continue;

                if (!debugSurfRenderer.Draw(cmd, *mesh, item.world))
                    break; // 항목 상수 한도를 넘었다: 나머지 항목도 그릴 수 없다
            }
        };
}