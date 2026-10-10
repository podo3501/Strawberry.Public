export module Pipeline.GraphBuilder:Opaque;

import std;
import :RenderRecordPool;
import :ViewTargetResources;
import Core.Assert;
import Pipeline.Renderer;
import DxRender.RGResourceID;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Resource;
import DxRender.Definition;
import DxRender.Command;
import DxRender.Task;
import Contract.Render.View;

export class OpaqueGraphBuilder
{
public:
    ~OpaqueGraphBuilder() = default;
    OpaqueGraphBuilder() = delete;
    OpaqueGraphBuilder(SurfaceRenderer& surfRenderer, DescriptorFactory& descFactory) :
        m_surfRenderer{ surfRenderer },
        m_descFactory{ descFactory },
        m_recordPool{ 10 }
    {}

    void Build(
        RenderGraph& graph,
        const DirectionalLightData& light,
        const ShadowResource& shadowRes,
        RGResourceID shadowResID,
        std::shared_ptr<SceneViewPacket> packet,
        const ViewTargetResources& target)
    {
        Core::Assert(!packet->surface.empty());

        auto& opaque = graph.AddGraphicsPass("Opaque_View" + std::to_string(packet->target.id));
        opaque.Read(shadowResID, RGAccess::SRV);
        opaque.Write(target.GetColorID(), RGAccess::RTV);
        opaque.Write(target.GetDepthID(), RGAccess::DepthWrite);
        opaque.numParallel = static_cast<std::uint32_t>(
            std::min<std::size_t>(3, std::max<std::size_t>(1, packet->surface.size())));

        opaque.execute =
            [
                &descFactory = m_descFactory,
                &surfRenderer = m_surfRenderer,
                &shadowRes = shadowRes,
                &recordPool = m_recordPool,
                light,
                packet,
                colorRTVIndex = target.GetColorRTVIndex(),
                depthDSVIndex = target.GetDepthDSVIndex()
            ]
        (TaskCommandLists cmds, TaskContext& ctx)
            {
                auto rtv = descFactory.GetRTVHandle(colorRTVIndex);
                auto dsv = descFactory.GetDSVHandle(depthDSVIndex);

                const std::size_t total = packet->surface.size();
                const std::size_t actual = std::min(cmds.Size(), total);
                const std::size_t chunkSize = (total + actual - 1) / actual;

                recordPool.ExecuteParallel(actual, [&](std::size_t t)
                    {
                        CommandList& cmd = cmds[t];
                        CommandUtils::SetRenderTarget(cmd, rtv, dsv);
                        CommandUtils::SetViewRect(cmd, packet->target.localViewport);

                        auto envRes = std::static_pointer_cast<EnvironmentResource>(packet->environment);
                        surfRenderer.PrepareDraw(
                            cmd, light, packet->target.camera,
                            shadowRes.GetSRVIndex(), envRes.get());

                        const std::size_t begin = t * chunkSize;
                        const std::size_t end = std::min(begin + chunkSize, total);
                        for (std::size_t i = begin; i < end; ++i)
                        {
                            auto& item = packet->surface[i];
                            auto mesh = static_cast<MeshResource*>(item.mesh.get());
                            auto material = static_cast<MaterialResource*>(item.material.get());
                            surfRenderer.BindPipeline(cmd, item.pipelineState);
                            surfRenderer.Draw(cmd, *mesh, *material, item.world);
                        }
                    });
            };
    }

private:
    SurfaceRenderer& m_surfRenderer;
    DescriptorFactory& m_descFactory;
    RenderRecordPool m_recordPool;
};