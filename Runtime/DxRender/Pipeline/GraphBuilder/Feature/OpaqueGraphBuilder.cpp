module DxRender.Pipeline.GraphBuilder;

import :Opaque;
import Core.Logger;
import Core.TypeHierarchy;
import DxRender.Command;
import DxRender.Resource;
import DxRender.Definition;

OpaqueGraphBuilder::OpaqueGraphBuilder(SurfaceRenderer& surfRenderer, DescriptorFactory& descFactory) :
    m_surfRenderer{ surfRenderer },
    m_descFactory{ descFactory },
    m_recordPool{ 10 }
{}

void OpaqueGraphBuilder::Build(
    RenderGraph& graph,
    const DirectionalLightData& light,
    const ShadowResource& shadowRes,
    RGResourceID shadowResID,
    std::shared_ptr<SceneViewPacket> packet,
    const ViewTargetResources& target)
{
    if (packet->surface.empty())
        return; // 그릴 것이 없으면 패스를 만들지 않는다

    auto& opaque = graph.AddGraphicsPass("Opaque_View" + std::to_string(packet->target.id));
    opaque.Read(shadowResID, RGAccess::SRV);
    opaque.Write(target.GetColorID(), RGAccess::RTV);
    opaque.Write(target.GetDepthID(), RGAccess::DepthWrite);
    opaque.numParallel = static_cast<std::uint32_t>(
        std::min<std::size_t>(MaxParallelRecordLists, std::max<std::size_t>(1, packet->surface.size())));

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

            // 환경이 있는데 환경 리소스가 아니면 환경 없이 그린다
            auto envRes = Core::Cast<EnvironmentResource>(packet->environment);
            if (packet->environment && !envRes)
                Core::Logger::Error("OpaqueGraphBuilder: 환경이 환경 리소스가 아니라 환경 없이 그린다");

            const std::size_t total = packet->surface.size();
            const std::size_t actual = std::min(cmds.Size(), total);
            if (actual == 0)
                return;
            const std::size_t chunkSize = (total + actual - 1) / actual;

            recordPool.ExecuteParallel(actual, [&](std::size_t t)
                {
                    CommandList& cmd = cmds[t];
                    CommandUtils::SetRenderTarget(cmd, rtv, dsv);
                    CommandUtils::SetViewRect(cmd, packet->target.localViewport);

                    if (!surfRenderer.PrepareDraw(
                        cmd, light, packet->target.camera,
                        shadowRes.GetSRVIndex(), envRes.get()))
                        return; // 프레임 상수 한도를 넘었다: 이 목록은 그리지 않는다

                    const std::size_t begin = t * chunkSize;
                    const std::size_t end = std::min(begin + chunkSize, total);
                    for (std::size_t i = begin; i < end; ++i)
                    {
                        auto& item = packet->surface[i];
                        auto mesh = Core::Cast<MeshResource>(item.mesh.get());
                        auto material = Core::Cast<MaterialResource>(item.material.get());
                        if (!mesh || !material)
                        {
                            Core::Logger::Error("OpaqueGraphBuilder: 메시나 재질이 올바른 리소스가 아니라 그리지 않는다");
                            continue;
                        }
                        if (!surfRenderer.BindPipeline(cmd, item.pipelineState))
                            continue;

                        if (!surfRenderer.Draw(cmd, *mesh, *material, item.world))
                            break; // 항목 상수 한도를 넘었다: 이 목록의 나머지 항목도 그릴 수 없다
                    }
                });
        };
}