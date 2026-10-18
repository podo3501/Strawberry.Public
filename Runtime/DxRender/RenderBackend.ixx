module;

#include <windows.h>

export module DxRender;

import std;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Command;
import DxRender.Definition;
import DxRender.SwapChainPresenter;
import DxRender.Shader;
import DxRender.Diagnostics;
import DxRender.Provider;
import DxRender.Text;
import DxRender.Pipeline;
import DxRender.Packet;
import DxRender.Task;
import Core.Math;
import Contract.Render.Types;
import Contract.Render.Interfaces;

class RenderBackend : public IRenderBackend
{
public:
    ~RenderBackend() override = default;
    explicit RenderBackend(const RenderConfig& config);

    virtual bool Initialize(HWND hwnd, const Core::Size& screenSize, const std::vector<RegistryShaderDesc>& registryShaders) override;
    virtual ShaderID RegisterShader(const ShaderDesc& desc) override;
    virtual void Resize(const Core::Size& size) override;
    virtual void Update() override;
    virtual void Render(SceneFrameData frame) override;
    virtual void Shutdown() override;

    virtual IResourceProvider* GetProvider(ProviderType type) override;
    virtual RenderMetrics GetRenderMetrics() override;

private:
    Device m_device;
    RenderConfig m_config;

    DescriptorFactory m_descFactory;
    ResourceFactory m_resFactory;
    CommandScheduler m_cmdScheduler;
    TaskScheduler m_taskScheduler;
    SwapChainPresenter m_swapChain;
    ShaderLibrary m_shaderLibrary;
    FrameProfiler m_profiler;
    ResourceProviderSet m_resProviderSet;
    TransientMeshProvider m_transientMeshProvider;
    TextSystem m_textSystem;
    ForwardRenderPipeline m_pipeline;

    std::uint64_t m_frameIndex{ 0 };
};