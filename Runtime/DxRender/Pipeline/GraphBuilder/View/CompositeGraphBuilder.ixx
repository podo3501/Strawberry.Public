export module Pipeline.GraphBuilder:Composite;

import std;
import :ViewTypes;
import DxRender.Graph;
import Pipeline.Renderer;
import DxRender.SwapChainPresenter;

export class CompositeGraphBuilder
{
public:
    CompositeGraphBuilder(
        CompositeRenderer& compositeRenderer,
        SwapChainPresenter& swapChain) noexcept;

    void Build(
        RenderGraph& graph,
        RGResourceID backBufferResID,
        const std::vector<ViewRenderOutput>& viewOutputs);

private:
    CompositeRenderer& m_compositeRenderer;
    SwapChainPresenter& m_swapChain;
};