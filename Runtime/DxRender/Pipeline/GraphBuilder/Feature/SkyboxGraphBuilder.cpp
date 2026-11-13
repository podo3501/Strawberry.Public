module DxRender.Pipeline.GraphBuilder;

import :Skybox;
import DxRender.Command;
import DxRender.Resource;
import Core.Logger;
import Core.TypeHierarchy;

SkyboxGraphBuilder::SkyboxGraphBuilder(SkyboxRenderer& skyboxRenderer, DescriptorFactory& descFactory) :
    m_skyboxRenderer{ skyboxRenderer },
    m_descFactory{ descFactory }
{}

void SkyboxGraphBuilder::Build(
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

            auto envRes = Core::Cast<EnvironmentResource>(packet->environment);
            if (packet->environment && !envRes)
            {
                Core::Logger::Error("SkyboxGraphBuilder: 환경이 환경 리소스가 아니라 스카이박스를 그리지 않는다");
                return;
            }
            if (!envRes || !envRes->IsReady())
                return; // 환경 없는 씬 - 스카이박스 안 그림

            auto skybox = envRes->GetSkybox();
            if (!skybox)
            {
                Core::Logger::Error("SkyboxGraphBuilder: 준비된 환경에 스카이박스 큐브가 없어 그리지 않는다");
                return;
            }

            auto rtv = descFactory.GetRTVHandle(colorRTVIndex);
            auto dsv = descFactory.GetDSVHandle(depthDSVIndex);

            CommandUtils::SetRenderTarget(cmd, rtv, dsv);
            CommandUtils::SetViewRect(cmd, packet->target.localViewport);

            if (!skyboxRenderer.Draw(cmd, packet->target.camera, *skybox))
                return; // 상수 버퍼 한도를 넘었다: 이 뷰의 스카이박스는 그리지 않는다
        };
}