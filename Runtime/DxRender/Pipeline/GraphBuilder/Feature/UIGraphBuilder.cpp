module DxRender.Pipeline.GraphBuilder;

import :UI;
import DxRender.Command;
import DxRender.Resource;
import Core.Logger;
import Core.TypeHierarchy;

UIGraphBuilder::UIGraphBuilder(UIRenderer& uiRenderer, DescriptorFactory& descFactory) :
    m_uiRenderer{ uiRenderer },
    m_descFactory{ descFactory }
{}

void UIGraphBuilder::Build(
    RenderGraph& graph,
    std::shared_ptr<OverlayViewPacket> packet,
    const ViewTargetResources& target)
{
    if (!packet->ui)
        return; // 그릴 UI가 없으면 패스를 만들지 않는다

    auto& ui = graph.AddGraphicsPass("UI_View" + std::to_string(packet->target.id));
    ui.Write(target.GetColorID(), RGAccess::RTV);

    ui.execute =
        [
            &descFactory = m_descFactory,
            &uiRenderer = m_uiRenderer,
            packet,
            colorRTVIndex = target.GetColorRTVIndex()
        ]
        (TaskCommandLists cmds, TaskContext& ctx)
        {
            auto mesh = Core::Cast<MeshResource>(packet->ui->mesh.get());
            if (!mesh)
            {
                Core::Logger::Error("UIGraphBuilder: UI 메시가 올바른 리소스가 아니라 그리지 않는다");
                return;
            }

            CommandList& cmd = cmds.Single();
            auto rtv = descFactory.GetRTVHandle(colorRTVIndex);

            CommandUtils::SetRenderTarget(cmd, rtv);
            CommandUtils::SetViewRect(cmd, packet->target.localViewport);

            uiRenderer.BeginFrame(cmd);

            Core::Matrix viewProj = packet->target.camera.view * packet->target.camera.proj;
            if (!uiRenderer.Draw(cmd, *mesh, viewProj))
                return; // 상수 버퍼 한도를 넘었다: 이 뷰의 UI는 그리지 않는다
        };
}