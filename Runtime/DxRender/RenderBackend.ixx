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
import Contract.Render.Backend;

class RenderBackend : public IRenderBackend
{
public:
    ~RenderBackend() override = default;
    explicit RenderBackend(const RenderConfig& config);

	bool Initialize(
        HWND hwnd, 
        const Core::Size& screenSize, 
        const std::vector<RegistryShaderDesc>& registryShaders) override;

	ShaderID RegisterShader(const ShaderDesc& desc) override;
	void Resize(const Core::Size& size) override;
	void Update() override;
	void Render(SceneFrameData frame) override;
	void Shutdown() override;

	IResourceProvider* GetProvider(ProviderType type) override;
	RenderMetrics GetRenderMetrics() override;

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