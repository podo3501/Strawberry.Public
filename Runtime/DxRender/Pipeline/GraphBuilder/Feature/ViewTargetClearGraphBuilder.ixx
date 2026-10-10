export module Pipeline.GraphBuilder:ViewTargetClear;

import :ViewTargetResources;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Command;
import DxRender.Task;

export class ViewTargetClearGraphBuilder
{
public:
    ~ViewTargetClearGraphBuilder() = default;

    explicit ViewTargetClearGraphBuilder(DescriptorFactory& descFactory) noexcept
        : m_descFactory{ descFactory }
    {
    }

    void Build(RenderGraph& graph, const ViewTargetResources& target)
    {
        auto& clear = graph.AddGraphicsPass("ClearViewTarget");
        clear.Write(target.GetColorID(), RGAccess::RTV);
        clear.Write(target.GetDepthID(), RGAccess::DepthWrite);

        clear.execute =
            [
                &descFactory = m_descFactory,
                colorRTVIndex = target.GetColorRTVIndex(),
                depthDSVIndex = target.GetDepthDSVIndex()
            ]
            (TaskCommandLists cmds, TaskContext& ctx)
            {
                CommandList& cmd = cmds.Single();

                auto rtv = descFactory.GetRTVHandle(colorRTVIndex);
                auto dsv = descFactory.GetDSVHandle(depthDSVIndex);

                float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // PMA 컨벤션: 완전 투명 = RGBA 모두 0
                CommandUtils::ClearRTV(cmd, rtv, clearColor);
                CommandUtils::ClearDSV(cmd, dsv, 1.0f);
            };
    }

private:
    DescriptorFactory& m_descFactory;
};