export module Pipeline.GraphBuilder:Composite;

import std;
import :ViewTypes;
import DxRender.RGResourceID;
import DxRender.Graph;
import Pipeline.Renderer;
import DxRender.SwapChainPresenter;
import DxRender.Command;
import DxRender.Task;

export class CompositeGraphBuilder
{
public:
    CompositeGraphBuilder(
        CompositeRenderer& compositeRenderer,
        SwapChainPresenter& swapChain) noexcept :
        m_compositeRenderer{ compositeRenderer },
        m_swapChain{ swapChain }
    {}

    void Build(
        RenderGraph& graph,
        RGResourceID backBufferResID,
        const std::vector<ViewRenderOutput>& viewOutputs)
    {
        auto& composite = graph.AddGraphicsPass("Composite");

        for (const auto& info : viewOutputs)
            composite.Read(info.colorID, RGAccess::SRV);
        composite.Write(backBufferResID, RGAccess::RTV);

        composite.execute =
            [
                &compositeRenderer = m_compositeRenderer,
                &swapChain = m_swapChain,
                viewOutputs
            ]
            (TaskCommandLists cmds, TaskContext& ctx)
            {
                CommandList& cmd = cmds.Single();
                swapChain.SetRenderTarget(cmd);

                compositeRenderer.PrepareDraw(cmd);
                for (const auto& info : viewOutputs)
                {
                    swapChain.SetViewport(cmd, info.viewport);
                    compositeRenderer.Draw(cmd, info.heapIndex);
                }
            };
    }

private:
    CompositeRenderer& m_compositeRenderer;
    SwapChainPresenter& m_swapChain;
};