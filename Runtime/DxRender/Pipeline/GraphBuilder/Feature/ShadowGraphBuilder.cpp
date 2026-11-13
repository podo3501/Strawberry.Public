module DxRender.Pipeline.GraphBuilder;

import :Shadow;
import DxRender.Command;
import DxRender.Resource;
import Contract.Render.Backend.Types;
import Core.Logger;
import Core.TypeHierarchy;

ShadowGraphBuilder::ShadowGraphBuilder(
    ShadowRenderer& shadowRenderer,
    DescriptorFactory& descFactory,
    ShadowResource& shadowRes) noexcept
    : m_shadowRenderer{ shadowRenderer }
    , m_descFactory{ descFactory }
    , m_shadowRes{ shadowRes }
{}

void ShadowGraphBuilder::Build(
    RenderGraph& graph,
    RGResourceID shadowResID,
    const DirectionalLightData& light,
    std::vector<RenderShadowCasterItem> shadowCasters)
{
    auto& shadow = graph.AddGraphicsPass("Shadow");
    shadow.Write(shadowResID, RGAccess::DepthWrite);

    shadow.execute =
        [
            &shadowRenderer = m_shadowRenderer,
            &descFactory = m_descFactory,
            &shadowRes = m_shadowRes,
            shadowCasters = std::move(shadowCasters),
            light
        ]
    (TaskCommandLists cmds, TaskContext& ctx)
        {
            CommandList& cmd = cmds.Single();
            if (shadowRes.GetWidth() == 0 || shadowRes.GetHeight() == 0)
            {
                Core::Logger::Error("ShadowGraphBuilder: 그림자 텍스처 크기를 알 수 없어 그림자를 그리지 않는다");
                return;
            }
            auto dsv = descFactory.GetDSVHandle(shadowRes.GetDSVIndex());

            CommandUtils::SetViewport(cmd, 0.f, 0.f, static_cast<float>(shadowRes.GetWidth()), static_cast<float>(shadowRes.GetHeight()));
            CommandUtils::SetScissor(cmd, 0, 0, static_cast<std::int32_t>(shadowRes.GetWidth()), static_cast<std::int32_t>(shadowRes.GetHeight()));

            CommandUtils::ClearDSV(cmd, dsv);
            CommandUtils::SetDepthTarget(cmd, dsv);

            if (!shadowRenderer.PrepareDraw(cmd, light))
                return; // 프레임 상수 한도를 넘었다: 그림자를 그리지 않는다

            for (auto& item : shadowCasters)
            {
                auto mesh = Core::Cast<MeshResource>(item.mesh.get());
                if (!mesh)
                {
                    Core::Logger::Error("ShadowGraphBuilder: 메시가 올바른 리소스가 아니라 그림자를 그리지 않는다");
                    continue;
                }
                if (!shadowRenderer.Draw(cmd, *mesh, item.world))
                    break; // 항목 상수 한도를 넘었다: 나머지 항목도 그릴 수 없다
            }
        };
}