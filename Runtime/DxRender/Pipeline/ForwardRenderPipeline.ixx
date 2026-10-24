module;

#include <d3d12.h>

export module DxRender.Pipeline:ForwardRender;

import std;
import DxRender.Core;
import DxRender.Definition;
import DxRender.Factory;
import DxRender.Command;
import DxRender.Graph;
import DxRender.Resource;
import DxRender.SwapChainPresenter;
import DxRender.Task;
import DxRender.Shader;
import DxRender.Pipeline.GraphBuilder;
import DxRender.Pipeline.Renderer;
import DxRender.Pipeline.Inspector;
import Core.Math;

export class ForwardRenderPipeline
{
public:
    ~ForwardRenderPipeline() = default;
    ForwardRenderPipeline(
        Device& device,
        SwapChainPresenter& swapChain,
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory,
        ShaderLibrary& shaderLibrary);

    bool Initialize(const Core::Size& screenSize);
    void Update();
    void Resize(const Core::Size& size);

    CommandList* Render(
        CommandList* cmd,
        CommandScheduler& cmdScheduler,
        FramePacket framePacket);

private:
    std::vector<CompiledTask> BuildFrame(const FramePacket& framePacket);

    Device& m_device;
    SwapChainPresenter& m_swapChain;
    DescriptorFactory& m_descFactory;

    RenderGraph m_graph;
    RGResourceIDAllocator m_idAllocator;

    Renderers m_renderers;

    ShadowResource m_shadowRes; //이 클래스는 framereseource 클래스중의 하나. 프레임당 render가 필요한 리소스들.
    InspectorRenderers m_inspectorRenderers;

    RGResourceID m_hBackBuffer{ InvalidRGID };
    RGResourceID m_hShadow{ InvalidRGID };

    FontAtlasUploadGraphBuilder m_fontUploadBuilder;
    ClearGraphBuilder m_clearBuilder;
    ShadowGraphBuilder m_shadowBuilder;
    ViewCompositionBuilder m_viewComposition;
    InspectorGraphBuilder m_inspectorBuilder;
};