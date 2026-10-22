export module Pipeline.GraphBuilder:Shadow;

import std;
import Pipeline.Renderer;
import DxRender.Definition;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Resource;
import Contract.Render.Backend.View;

export class ShadowGraphBuilder
{
public:
    ~ShadowGraphBuilder() = default;

    ShadowGraphBuilder(
        ShadowRenderer& shadowRenderer,
        DescriptorFactory& descFactory,
        ShadowResource& shadowRes) noexcept;

    void Build(
        RenderGraph& graph,
        RGResourceID shadowResID,
        const DirectionalLightData& light,
        std::vector<RenderShadowCasterItem> shadowCasters);

private:
    ShadowRenderer& m_shadowRenderer;
    DescriptorFactory& m_descFactory;
    ShadowResource& m_shadowRes;
};