export module Pipeline.GraphBuilder:ViewTargetClear;

import :ViewTargetResources;
import DxRender.Graph;
import DxRender.Factory;

export class ViewTargetClearGraphBuilder
{
public:
    ~ViewTargetClearGraphBuilder() = default;
    explicit ViewTargetClearGraphBuilder(DescriptorFactory& descFactory) noexcept;

    void Build(RenderGraph& graph, const ViewTargetResources& target);

private:
    DescriptorFactory& m_descFactory;
};