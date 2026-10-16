//DirectX관련
#pragma comment(lib, "dxcompiler.lib") //셰이더 관련
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

//text 관련
#ifdef _DEBUG
#pragma comment(lib, "freetype_Debug.lib")
#pragma comment(lib, "harfbuzz_Debug.lib")
#pragma comment(lib, "msdfgen-core_Debug.lib")
#pragma comment(lib, "msdfgen-ext_Debug.lib")
#else
#pragma comment(lib, "freetype_Release.lib")
#pragma comment(lib, "harfbuzz_Release.lib")
#pragma comment(lib, "msdfgen-core_Release.lib")
#pragma comment(lib, "msdfgen-ext_Release.lib")
#endif

//cube texture
#ifdef _DEBUG
#pragma comment(lib, "ktx_Debug.lib")
#else
#pragma comment(lib, "ktx_Release.lib")
#endif

module DxRender;

import DxRender.Packet;
import DxRender.Definition;

RenderBackend::RenderBackend(const RenderConfig& config)
    : m_device{ config.enableDebugLayer }
    , m_config{ config }
    , m_descFactory{ m_device }
    , m_resFactory{ m_device }
    , m_taskScheduler{ m_cmdScheduler }
    , m_swapChain{ m_cmdScheduler }
    , m_resProviderSet{ m_device, m_taskScheduler, m_resFactory, m_descFactory }
    , m_transientMeshProvider{ m_descFactory }
    , m_textSystem{ m_device, m_descFactory, m_resFactory }
    , m_pipeline{ m_device, m_swapChain, m_taskScheduler, m_descFactory, m_shaderLibrary, m_textSystem.GetBuilder() }
{}

bool RenderBackend::Initialize(HWND hwnd, const Core::Size& screenSize, std::span<const RegistryShaderDesc> registryShaders)
{
    if (!m_descFactory.Initialize(m_config.descriptors)) return false;
    if (!m_cmdScheduler.Initialize(m_device, m_descFactory.GetBindlessAllocator().GetHeap(), m_config.commandPools)) return false;
    SwapChainDesc desc{ hwnd, screenSize, m_config.allowTearing };
    if (!m_swapChain.Initialize(m_device, desc)) return false;
    if (!m_shaderLibrary.Initialize(registryShaders)) return false;
    if (!m_profiler.Initialize(m_device, m_cmdScheduler, m_resFactory)) return false;
    if (!m_resProviderSet.Initialize(m_shaderLibrary)) return false;
    if (!m_transientMeshProvider.Initialize(m_device)) return false;
    if (!m_textSystem.Initialize(m_config.text)) return false;
    if (!m_pipeline.Initialize(screenSize)) return false;

    return true;
}

ShaderID RenderBackend::RegisterShader(const ShaderDesc& desc)
{
    return m_shaderLibrary.RegisterShader(desc);
}

void RenderBackend::Resize(const Core::Size& size)
{
    m_swapChain.Resize(m_device, size);
    m_pipeline.Resize(size);
}

void RenderBackend::Update()
{
    m_taskScheduler.Execute();

    m_profiler.Update(m_frameIndex);
    float gpuMs = m_profiler.GetGpuFrameTimeMs();

    m_resProviderSet.Update(gpuMs);
    m_pipeline.Update();
}

void RenderBackend::Render(SceneFrameData frame)
{
    std::uint32_t slot = static_cast<std::uint32_t>(m_frameIndex % FrameBufferCount);
    auto* cmd = m_cmdScheduler.Begin(slot);
    if (cmd)
    {
        m_transientMeshProvider.ResetFrame(slot);
        m_descFactory.BeginFrame(slot);

        m_profiler.BeginFrame(*cmd, m_frameIndex);

        cmd = m_pipeline.Render(cmd, m_cmdScheduler,
            BuildPacket(
                frame,
                m_textSystem,
                m_transientMeshProvider,
                m_swapChain.GetSize()));

        if (!cmd)
        {
            m_cmdScheduler.AbortFrame();
            return;
        }

        m_profiler.EndFrame(*cmd);

        m_cmdScheduler.End();
        m_swapChain.Present(false);

        m_frameIndex++;
    }
}

void RenderBackend::Shutdown()
{
    m_taskScheduler.Shutdown(); // 일감을 다 끝내놓는다.
    m_cmdScheduler.WaitIdle();  // 초기화 같은 것들은 task에 안 들어가기 때문에 cmd가 끝나기를 기다린다.
}

IResourceProvider* RenderBackend::GetProvider(ProviderType type)
{
    return m_resProviderSet.GetProvider(type);
}

RenderMetrics RenderBackend::GetRenderMetrics()
{
    RenderMetrics metrics;

    metrics.cpuFrameMs = m_profiler.GetCpuFrameTimeMs();
    metrics.gpuFrameMs = m_profiler.GetGpuFrameTimeMs();

    return metrics;
}

std::unique_ptr<IRenderBackend> CreateRenderBackend(const RenderConfig& config)
{
    return std::make_unique<RenderBackend>(config);
}