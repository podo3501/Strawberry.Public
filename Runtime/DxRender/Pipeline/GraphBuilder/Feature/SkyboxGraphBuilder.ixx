export module Pipeline.GraphBuilder:Skybox;

import :ViewTargetResources;
import std;
import Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Definition;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Task;

export class SkyboxGraphBuilder
{
public:
    ~SkyboxGraphBuilder() = default;
    SkyboxGraphBuilder() = delete;
    SkyboxGraphBuilder(SkyboxRenderer& skyboxRenderer, DescriptorFactory& descFactory) :
        m_skyboxRenderer{ skyboxRenderer },
        m_descFactory{ descFactory }
    {}

    void Build(
        RenderGraph& graph,
        std::shared_ptr<SceneViewPacket> packet,
        const ViewTargetResources& target)
    {
        auto& skybox = graph.AddGraphicsPass("Skybox_View" + std::to_string(packet->target.id));
        skybox.Write(target.GetColorID(), RGAccess::RTV);
        skybox.Write(target.GetDepthID(), RGAccess::DepthWrite);

        skybox.execute =
            [
                &descFactory = m_descFactory,
                &skyboxRenderer = m_skyboxRenderer,
                packet,
                colorRTVIndex = target.GetColorRTVIndex(),
                depthDSVIndex = target.GetDepthDSVIndex()
            ]
        (TaskCommandLists cmds, TaskContext& ctx)
            {
                CommandList& cmd = cmds.Single();

                auto envRes = std::static_pointer_cast<EnvironmentResource>(packet->environment);
                if (!envRes || !envRes->IsReady())
                    return; // 환경 없는 씬 - 스카이박스 안 그림

                auto rtv = descFactory.GetRTVHandle(colorRTVIndex);
                auto dsv = descFactory.GetDSVHandle(depthDSVIndex);

                CommandUtils::SetRenderTarget(cmd, rtv, dsv);
                CommandUtils::SetViewRect(cmd, packet->target.localViewport);

                skyboxRenderer.Draw(cmd, packet->target.camera, *envRes->GetSkybox());
            };
    }

private:
    SkyboxRenderer& m_skyboxRenderer;
    DescriptorFactory& m_descFactory;
};