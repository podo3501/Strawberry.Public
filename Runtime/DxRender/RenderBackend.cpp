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

import std;
import Contract.Render.Backend.Types;
import Core.Assert;
import Core.Logger;
import Core.TypeHierarchy;

RenderBackend::RenderBackend(const RenderConfig& config)
    : m_config{ config }
    , m_descFactory{ m_device }
    , m_resFactory{ m_device }
    , m_taskScheduler{ m_cmdScheduler }
    , m_swapChain{ m_cmdScheduler }
    , m_resProviderSet{ m_device, m_taskScheduler, m_resFactory, m_descFactory }
    , m_transientMesh{ m_descFactory }
    , m_textSystem{ m_device, m_descFactory }
    , m_pipeline{ m_device, m_swapChain, m_taskScheduler, m_resFactory, m_descFactory, m_shaderLibrary }
{}

bool RenderBackend::Initialize(HWND hwnd, const Core::Size& screenSize, const std::vector<RegistryShaderDesc>& registryShaders)
{
    // 실패한 단계를 한 줄로 남기고 false를 돌려준다 (자세한 원인은 그 단계가 먼저 남긴다).
    const auto fail = [](std::string_view step)
    {
        Core::Logger::Error("RenderBackend::Initialize: {} 초기화에 실패했다", step);
        return false;
    };

    const AdapterKind adapter = m_config.adapter == RenderAdapter::Warp ? AdapterKind::Warp : AdapterKind::Auto;
    if (!m_device.Initialize({ .debug = { .enableDebugLayer = m_config.enableDebugLayer }, .adapter = adapter })) return fail("디바이스");
    if (!m_descFactory.Initialize(m_config.descriptors)) return fail("디스크립터");
    if (!m_cmdScheduler.Initialize(m_device, m_descFactory.GetBindlessAllocator().GetHeap(), m_config.commandPools)) return fail("명령 스케줄러");
    SwapChainDesc desc{ hwnd, screenSize, m_config.allowTearing };
    if (!m_swapChain.Initialize(m_device, desc)) return fail("스왑 체인");
    if (!m_shaderLibrary.Initialize(registryShaders)) return fail("셰이더 라이브러리");
    if (!m_profiler.Initialize(m_device, m_cmdScheduler, m_resFactory)) return fail("프레임 프로파일러");
    if (!m_resProviderSet.Initialize(m_shaderLibrary)) return fail("리소스 프로바이더");
    if (!m_transientMesh.Initialize(m_device)) return fail("일시 메시");
    if (!m_textSystem.Initialize(m_config.text)) return fail("글자 시스템");
    if (!m_pipeline.Initialize(screenSize)) return fail("렌더 파이프라인");

    return true;
}

ShaderID RenderBackend::RegisterShader(const ShaderDesc& desc)
{
    return m_shaderLibrary.RegisterShader(desc);
}

bool RenderBackend::Resize(const Core::Size& size)
{
    if (!m_swapChain.Resize(m_device, size)) return false; // 파이프라인은 이전 크기를 유지한다

    m_pipeline.Resize(size);
    return true;
}

void RenderBackend::Update()
{
    m_taskScheduler.Execute();

    m_profiler.Update(m_frameIndex);
    float gpuMs = m_profiler.GetGpuFrameTimeMs();

    m_resProviderSet.Update(gpuMs);
    m_pipeline.Update();
}

bool RenderBackend::Render(SceneFrameData frame)
{
    if (!m_swapChain.IsReady()) // 크기 변경에 실패했거나 장치를 잃은 뒤에는 그릴 백버퍼가 없다
    {
        if (!m_notReadyLogged)
            Core::Logger::Error("RenderBackend::Render: 그릴 수 있는 화면이 없어 프레임을 건너뛴다 (크기 변경 실패나 장치 손실 뒤)");
        m_notReadyLogged = true;
        return false;
    }
    m_notReadyLogged = false;

    std::uint32_t slot = static_cast<std::uint32_t>(m_frameIndex % FrameBufferCount);
    auto* cmd = m_cmdScheduler.Begin(slot);
    if (!cmd)
        return true; // 명령 목록이 모자라면 기다리지 않고 이번 프레임만 건너뛴다 (실패가 아니다)

    m_transientMesh.ResetFrame(slot);
    m_descFactory.BeginFrame(slot);

    m_profiler.BeginFrame(*cmd, m_frameIndex);

    cmd = m_pipeline.Render(cmd, m_cmdScheduler,
        BuildPacket(
            frame,
            m_textSystem,
            m_transientMesh,
            m_swapChain.GetSize()));

    if (!cmd)
    {
        Core::Logger::Error("RenderBackend::Render: 렌더 파이프라인이 프레임을 끝내지 못해 이번 프레임을 버린다");
        m_cmdScheduler.AbortFrame();
        return false;
    }

    if (m_swapChain.IsOffscreen())
        m_swapChain.RecordReadback(*cmd);

    m_profiler.EndFrame(*cmd);

    m_profiler.MarkSubmitted(m_cmdScheduler.End());

    m_frameIndex++; // 제출한 프레임이므로 Present가 실패해도 슬롯은 넘어간다
    return m_swapChain.Present(false);
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

Core::Vector2 RenderBackend::MeasureText(
    const std::shared_ptr<IResource>& font,
    const std::vector<TextRun>& runs,
    std::uint32_t size,
    float maxWidth,
    const TextLayout& layout)
{
    FontResource* fontRes = Core::Cast<FontResource>(font.get());
    if (!fontRes)
    {
        Core::Logger::Error("RenderBackend::MeasureText: 폰트가 없거나 폰트 리소스가 아니다");
        return Core::Vector2{};
    }
    return MeasureRuns(*fontRes, runs, size, maxWidth, layout);
}

RenderMetrics RenderBackend::GetRenderMetrics()
{
    RenderMetrics metrics;

    metrics.cpuFrameMs = m_profiler.GetCpuFrameTimeMs();
    metrics.gpuFrameMs = m_profiler.GetGpuFrameTimeMs();

    return metrics;
}

bool RenderBackend::ReadbackFrame(FrameImage& out)
{
    if (!m_swapChain.IsOffscreen())
    {
        Core::Logger::Error("RenderBackend::ReadbackFrame: 오프스크린 모드가 아니라 화면을 읽을 수 없다");
        return false;
    }
    if (!m_swapChain.ReadPixels(out.pixels)) return false; // 원인은 ReadPixels가 남긴다

    out.size = m_swapChain.GetSize();
    return true;
}

std::unique_ptr<IRenderBackend> CreateRenderBackend(const RenderConfig& config)
{
    return std::make_unique<RenderBackend>(config);
}