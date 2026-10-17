export module Pipeline.GraphBuilder:OverlayView;

import :ViewTypes;
import :ViewTargetClear;
import :UI;
import Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Resource;

export class OverlayViewGraphBuilder
{
public:
    OverlayViewGraphBuilder(
        DescriptorFactory& descFactory,
        Renderers& renderers);

    ViewRenderOutput Build(
        RenderGraph& graph,
        const ViewTargetResources& target,
        const std::shared_ptr<OverlayViewPacket>& view);

private:
    ViewTargetClearGraphBuilder m_clearBuilder;
    UIGraphBuilder m_uiBuilder;
};