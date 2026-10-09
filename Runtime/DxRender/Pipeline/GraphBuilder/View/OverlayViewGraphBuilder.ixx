export module Pipeline.GraphBuilder:OverlayView;

import :ViewTargetClear;
import :UI;
import Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Resource;
import DxRender.Definition;

export class OverlayViewGraphBuilder
{
public:
    OverlayViewGraphBuilder(
        DescriptorFactory& descFactory,
        Renderers& renderers) :
        m_clearBuilder{ descFactory },
        m_uiBuilder{ renderers.GetUIRenderer(), descFactory }
    {}

    ViewRenderOutput Build(
        RenderGraph& graph,
        const ViewTargetResource& target,
        const std::shared_ptr<OverlayViewPacket>& view)
    {
        graph.ImportResource(target.GetColorID(), RGAccess::SRV);
        graph.ImportResource(target.GetDepthID(), RGAccess::DepthWrite);

        m_clearBuilder.Build(graph, target); // Overlay는 보통 투명 배경으로 clear

        if (view->ui)
            m_uiBuilder.Build(graph, view, target);

        graph.ExportResource(target.GetColorID(), RGAccess::SRV);
        graph.ExportResource(target.GetDepthID(), RGAccess::DepthWrite);

        return { view->target.id, view->target.viewport, target.GetHeapIndex(), target.GetColorID() };
    }

private:
    ViewTargetClearGraphBuilder m_clearBuilder;
    UIGraphBuilder m_uiBuilder;
};