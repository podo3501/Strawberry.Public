export module DxRender.Pipeline.GraphBuilder:UI;

import std;
import :ViewTargetResources;
import DxRender.Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Definition;

export class UIGraphBuilder
{
public:
    ~UIGraphBuilder() = default;
    UIGraphBuilder() = delete;
    UIGraphBuilder(UIRenderer& uiRenderer, DescriptorFactory& descFactory);

    void Build(
        RenderGraph& graph,
        std::shared_ptr<OverlayViewPacket> packet,
        const ViewTargetResources& target);

private:
    UIRenderer& m_uiRenderer;
    DescriptorFactory& m_descFactory;
};