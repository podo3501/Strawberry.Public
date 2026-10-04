module;

#include <d3d12.h>

export module Runtime.Render.Pipeline:ForwardRender;

import std;
import Runtime.Render.Core;
import Runtime.Render.Factory;
import Runtime.Render.Command;
import Runtime.Render.Helper;
import Runtime.Render.Definition;
import Runtime.Render.RGResourceID;
import Runtime.Render.Graph;
import Runtime.Render.Resource;
import Runtime.Render.SwapChainPresenter;
import Runtime.Render.Task;
import Runtime.Render.Shader;
import Pipeline.GraphBuilder;
import Pipeline.Renderer;
import Pipeline.Inspector;
import Core.Math;

namespace
{
    constexpr Core::Size ShadowMapSize = { 2048, 2048 };

    Resource CreateShadowMap(Device& device)
    {
        auto desc = CreateTextureDescriptor(ShadowMapSize.width, ShadowMapSize.height, DXGI_FORMAT_R32_TYPELESS);
        desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format = RenderFormat::ShadowMapFormat;
        clearValue.DepthStencil.Depth = 1.0f;

        return device.CreateResource(desc, D3D12_HEAP_TYPE_DEFAULT,
            D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue);
    }
} //namespace

export class ForwardRenderPipeline
{
public:
    ~ForwardRenderPipeline() = default;

    ForwardRenderPipeline(
        Device& device,
        SwapChainPresenter& swapChain,
        TaskScheduler& taskScheduler,
        DescriptorFactory& descFactory,
        ShaderLibrary& shaderLibrary,
        FontAtlasUploadGraphBuilder& fontUploadBuilder)
        : m_device{ device }
        , m_swapChain{ swapChain }
        , m_descFactory{ descFactory }
        , m_renderers{ device, shaderLibrary }
        , m_viewComposition{ device, taskScheduler, descFactory, m_renderers, swapChain, m_graph, m_idAllocator }
        , m_inspectorRenderers{ device, shaderLibrary }
        , m_fontUploadBuilder{ fontUploadBuilder }
        , m_clearBuilder{ m_swapChain }
        , m_shadowBuilder{ m_renderers.GetShadowRenderer(), descFactory, m_shadowRes }
        , m_inspectorBuilder{ m_inspectorRenderers.GetInspectorImageRenderer(), descFactory, m_swapChain }
    {
    }

    bool Initialize(const Core::Size& screenSize)
    {
        Resource shadow = CreateShadowMap(m_device);
        if (!shadow) return false;

        UINT dsv = m_descFactory.CreateTextureDSV(shadow, DXGI_FORMAT_D32_FLOAT);
        UINT srv = m_descFactory.CreateTextureSRV(shadow, DXGI_FORMAT_R32_FLOAT);

        if (!m_shadowRes.Initialize(std::move(shadow), dsv, srv)) return false;
        if (!m_renderers.Initialize()) return false;
        if (!m_inspectorRenderers.Initialize(screenSize)) return false;

        m_hBackBuffer = m_idAllocator.AllocatePersistent();
        m_hShadow = m_idAllocator.AllocatePersistent();

        return true;
    }

    void Update()
    {
        m_viewComposition.Update();
    }

    CommandList* Render(
        CommandList* cmd,
        CommandScheduler& cmdScheduler,
        FramePacket framePacket)
    {
        auto compiledTasks = BuildFrame(framePacket); // 매 프레임 그래프 재구성

        TaskContext ctx;
        ctx.resources = std::make_shared<ResourceContext>(TotalResourceIDCapacity);

        m_fontUploadBuilder.ApplyResourceBindings(*ctx.resources);
        m_viewComposition.ApplyResourceBindings(*ctx.resources);
        ctx.SetResource(m_hBackBuffer, m_swapChain.GetCurrentBackbuffer());
        ctx.SetResource(m_hShadow, m_shadowRes.GetResource());

        return ExecuteRenderPipeline(cmd, cmdScheduler, compiledTasks, ctx);
    }

    void Resize(const Core::Size& size)
    {
        m_inspectorRenderers.SetScreenSize(size);
    }

private:
    std::vector<CompiledTask> BuildFrame(const FramePacket& framePacket)
    {
        m_renderers.ResetFrameResources(m_descFactory.GetCurrentSlot()); // 이전 프레임에 썼던 데이터들을 초기화.
        m_graph.Reset();
        m_idAllocator.ResetTransient();

        m_graph.ImportResource(m_hBackBuffer, RGAccess::Present);
        m_graph.ImportResource(m_hShadow, RGAccess::DepthWrite);

        if (m_fontUploadBuilder.HasPendingUploads())
            m_fontUploadBuilder.Build(m_graph, m_idAllocator);

        m_clearBuilder.Build(m_graph, m_hBackBuffer);
        m_shadowBuilder.Build(m_graph, m_hShadow, framePacket.light, framePacket.shadowCasters);

        FramePassContext frameCtx{ framePacket.light, m_shadowRes, m_hShadow };
        m_viewComposition.Build(m_hBackBuffer, framePacket, frameCtx);

        // shadow map은 텍스처가 크기 때문에 작은 물체를 띄우면 안 보인다.
        // m_inspectorBuilder.Build(m_graph, m_hBackBuffer, 10); // 인자는 보고 싶은 srv Index(Heap Index)를 넣으면 된다.

        m_graph.ExportResource(m_hBackBuffer, RGAccess::Present);
        m_graph.ExportResource(m_hShadow, RGAccess::DepthWrite);

        return m_graph.Compile();
    }

private:
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

    FontAtlasUploadGraphBuilder& m_fontUploadBuilder;
    ClearGraphBuilder m_clearBuilder;
    ShadowGraphBuilder m_shadowBuilder;
    ViewCompositionBuilder m_viewComposition;
    InspectorGraphBuilder m_inspectorBuilder;
};