export module DxRender.Pipeline.GraphBuilder:Clear;

import DxRender.SwapChainPresenter;
import DxRender.Graph;

export class ClearGraphBuilder
{
public:
    ~ClearGraphBuilder() = default;
    ClearGraphBuilder() = delete;

    explicit ClearGraphBuilder(SwapChainPresenter& swapChain) noexcept;
    void Build(RenderGraph& graph, RGResourceID backBufferResID);

private:
    SwapChainPresenter& m_swapChain;
};