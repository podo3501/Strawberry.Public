export module DxRender.Pipeline.Inspector:GraphBuilder;

import std;
import :ImageRenderer;
import DxRender.Factory;
import DxRender.SwapChainPresenter;
import DxRender.Graph;

export class InspectorGraphBuilder
{
public:
    ~InspectorGraphBuilder() = default;
    InspectorGraphBuilder() = delete;
    InspectorGraphBuilder(
        InspectorImageRenderer& imageRenderer,
        DescriptorFactory& descFactory,
        SwapChainPresenter& swapChain) noexcept;

    void Build(
        RenderGraph& graph,
        RGResourceID backBufferResID,
        std::uint32_t srvIndex);

private:
    InspectorImageRenderer& m_imageRenderer;
    DescriptorFactory& m_descFactory;
    SwapChainPresenter& m_swapChain;

    RGResourceID m_backBufferResID{ InvalidRGID };
};