export module Pipeline.Inspector:GraphBuilder;

import std;
import :ImageRenderer;
import Runtime.Render.Factory;
import Runtime.Render.SwapChainPresenter;
import Runtime.Render.RGResourceID;
import Runtime.Render.Graph;
import Runtime.Render.Task;

export class InspectorGraphBuilder
{
public:
    ~InspectorGraphBuilder() = default;
    InspectorGraphBuilder() = delete;
    InspectorGraphBuilder(
        InspectorImageRenderer& imageRenderer,
        DescriptorFactory& descFactory,
        SwapChainPresenter& swapChain) noexcept :
        m_imageRenderer{ imageRenderer },
        m_descFactory{ descFactory },
        m_swapChain{ swapChain }
    {}

    void Build(
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

private:
    InspectorImageRenderer& m_imageRenderer;
    DescriptorFactory& m_descFactory;
    SwapChainPresenter& m_swapChain;

    RGResourceID m_backBufferResID{ InvalidRGID };
};