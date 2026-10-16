export module Pipeline.GraphBuilder:Skybox;

import std;
import :ViewTargetResources;
import Pipeline.Renderer;
import DxRender.Graph;
import DxRender.Factory;
import DxRender.Definition;

export class SkyboxGraphBuilder
{
public:
    ~SkyboxGraphBuilder() = default;
    SkyboxGraphBuilder() = delete;
    SkyboxGraphBuilder(SkyboxRenderer& skyboxRenderer, DescriptorFactory& descFactory);

    void Build(
        RenderGraph& graph,
        std::shared_ptr<SceneViewPacket> packet,
        const ViewTargetResources& target);

private:
    SkyboxRenderer& m_skyboxRenderer;
    DescriptorFactory& m_descFactory;
};