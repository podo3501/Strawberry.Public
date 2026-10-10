export module Pipeline.GraphBuilder:UI;

import std;
import :ViewTargetResources;
import Core.Assert;
import Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Definition;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Task;

export class UIGraphBuilder
{
public:
    ~UIGraphBuilder() = default;
    UIGraphBuilder() = delete;

    UIGraphBuilder(UIRenderer& uiRenderer, DescriptorFactory& descFactory) :
        m_uiRenderer{ uiRenderer },
        m_descFactory{ descFactory }
    {}

    void Build(
        RenderGraph& graph,
        std::shared_ptr<OverlayViewPacket> packet,
        const ViewTargetResources& target)
    {
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
                Core::Assert(packet->ui.has_value());

                CommandList& cmd = cmds.Single();
                auto rtv = descFactory.GetRTVHandle(colorRTVIndex);

                CommandUtils::SetRenderTarget(cmd, rtv);
                CommandUtils::SetViewRect(cmd, packet->target.localViewport);

                uiRenderer.BeginFrame(cmd);

                auto mesh = static_cast<MeshResource*>(packet->ui->mesh.get());
                Core::Matrix viewProj = packet->target.camera.view * packet->target.camera.proj;
                uiRenderer.Draw(cmd, *mesh, viewProj);
            };
    }

private:
    UIRenderer& m_uiRenderer;
    DescriptorFactory& m_descFactory;
};