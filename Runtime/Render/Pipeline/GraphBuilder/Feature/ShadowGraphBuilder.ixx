export module Pipeline.GraphBuilder:Shadow;

import std;
import Pipeline.Renderer;
import Runtime.Render.Definition;
import Runtime.Render.RGResourceID;
import Runtime.Render.Graph;
import Runtime.Render.Factory;
import Runtime.Render.Resource;
import Runtime.Render.Command;
import Client.Render.Definition;
import Client.Render.View;

export class ShadowGraphBuilder
{
public:
    ~ShadowGraphBuilder() = default;

    ShadowGraphBuilder(
        ShadowRenderer& shadowRenderer,
        DescriptorFactory& descFactory,
        ShadowResource& shadowRes) noexcept
        : m_shadowRenderer{ shadowRenderer }
        , m_descFactory{ descFactory }
        , m_shadowRes{ shadowRes }
    {}

    void Build(
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
                auto dsv = descFactory.GetDSVHandle(shadowRes.GetDSVIndex());

                CommandUtils::SetViewport(cmd, 0.f, 0.f, 2048.f, 2048.f);
                CommandUtils::SetScissor(cmd, 0, 0, 2048, 2048);

                CommandUtils::ClearDSV(cmd, dsv);
                CommandUtils::SetDepthTarget(cmd, dsv);

                shadowRenderer.PrepareDraw(cmd, light);

                for (auto& item : shadowCasters)
                {
                    auto mesh = static_cast<MeshResource*>(item.mesh.get());
                    shadowRenderer.Draw(cmd, *mesh, item.world);
                }
            };
    }

private:
    ShadowRenderer& m_shadowRenderer;
    DescriptorFactory& m_descFactory;
    ShadowResource& m_shadowRes;
};