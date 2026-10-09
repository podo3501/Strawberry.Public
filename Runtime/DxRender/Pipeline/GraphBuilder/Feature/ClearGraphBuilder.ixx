export module Pipeline.GraphBuilder:Clear;

import DxRender.SwapChainPresenter;
import DxRender.RGResourceID;
import DxRender.Graph;
import DxRender.Task;
import DxRender.Command;

export class ClearGraphBuilder
{
public:
    ~ClearGraphBuilder() = default;
    ClearGraphBuilder() = delete;
    explicit ClearGraphBuilder(SwapChainPresenter& swapChain) noexcept
        : m_swapChain{ swapChain }
    {
    }

    void Build(RenderGraph& graph, RGResourceID backBufferResID)
    {
        auto& clear = graph.AddGraphicsPass("ClearBackBuffer");
        clear.Write(backBufferResID, RGAccess::RTV);
        clear.execute =
            [
                &swapChain = m_swapChain
            ]
            (TaskCommandLists cmds, TaskContext& ctx)
            {
                CommandList& cmd = cmds.Single();

                swapChain.SetRenderTarget(cmd);
                swapChain.Clear(cmd, 0.13f, 0.13f, 0.16f, 1.0f);
            };
    }

private:
    SwapChainPresenter& m_swapChain;
};