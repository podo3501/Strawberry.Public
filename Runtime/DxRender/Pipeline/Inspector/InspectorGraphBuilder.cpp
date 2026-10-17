module Pipeline.Inspector;

import :GraphBuilder;

InspectorGraphBuilder::InspectorGraphBuilder(
    InspectorImageRenderer& imageRenderer,
    DescriptorFactory& descFactory,
    SwapChainPresenter& swapChain) noexcept :
    m_imageRenderer{ imageRenderer },
    m_descFactory{ descFactory },
    m_swapChain{ swapChain }
{}

void InspectorGraphBuilder::Build(
    RenderGraph& graph,
    RGResourceID backBufferResID,
    std::uint32_t srvIndex)
{
    auto& inspector = graph.AddGraphicsPass("Inspector");
    inspector.Write(backBufferResID, RGAccess::RTV);
    inspector.execute =
        [
            &imageInspector = m_imageRenderer,
            &descFactory = m_descFactory,
            &swapChain = m_swapChain,
            srvIndex
        ]
        (TaskCommandLists cmds, TaskContext& ctx)
        {
            CommandList& cmd = cmds.Single();

            swapChain.SetRenderTarget(cmd);
            swapChain.SetViewport(cmd);

            imageInspector.PrepareFrame(descFactory.GetCurrentSlot());
            imageInspector.BeginFrame(cmd);

            imageInspector.BindPipeline(cmd);
            imageInspector.Draw(cmd, srvIndex);
        };
}