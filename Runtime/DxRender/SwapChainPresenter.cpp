module;

#include <dxgi1_6.h>
#include <wrl.h>
#include "d3dx12.h"

module DxRender.SwapChainPresenter;

import Core.Logger;
import DxRender.Definition;

SwapChainPresenter::~SwapChainPresenter()
{
    m_cmdScheduler.WaitIdle(CommandType::Direct);
}

SwapChainPresenter::SwapChainPresenter(CommandScheduler& cmdScheduler)
    : m_cmdScheduler{ cmdScheduler }
{}

bool SwapChainPresenter::Initialize(Device& device, const SwapChainDesc& desc)
{
    if (desc.size.width == 0 || desc.size.height == 0)
    {
        Core::Logger::Error("SwapChainPresenter::Initialize: 크기가 0이다 ({}x{})", desc.size.width, desc.size.height);
        return false;
    }

    if (desc.frameCount == 0)
    {
        Core::Logger::Error("SwapChainPresenter::Initialize: frameCount가 0이다");
        return false;
    }

    m_frameCount = desc.frameCount;
    m_renderTargets.resize(m_frameCount);

    m_size = desc.size;
    m_tearing = desc.allowTearing;
    m_offscreen = (desc.hwnd == nullptr);

    auto queue = m_cmdScheduler.GetQueue(CommandType::Direct)->GetQueue();
    if (desc.hwnd && !CreateSwapChain(device, queue, desc)) return false;

    if (!CreateRTV(device)) return false;
    if (!CreateDSV(device)) return false;
    if (!CreateDepthBuffer(device)) return false;
    if (IsOffscreen() && !CreateReadbackBuffer(device)) return false;

    m_frameIndex = m_swapChain ? m_swapChain->GetCurrentBackBufferIndex() : 0;

    m_ready = true;
    return true;
}

void SwapChainPresenter::Clear(CommandList& cmd, float r, float g, float b, float a)
{
    float color[4] = { r, g, b, a };
    CommandUtils::ClearRTV(cmd, GetCurrentRTV(), color);
    CommandUtils::ClearDSV(cmd, GetDSV());
}

void SwapChainPresenter::SetRenderTarget(CommandList& cmd)
{
    CommandUtils::SetRenderTarget(cmd, GetCurrentRTV(), GetDSV());
}

void SwapChainPresenter::SetViewport(CommandList& cmd, const std::optional<Core::Rect>& viewport)
{
    const Core::Rect rect = viewport.value_or(Core::Rect{
        0.f, 0.f,
        static_cast<float>(m_size.width),
        static_cast<float>(m_size.height)
        });

    CommandUtils::SetViewport(cmd, rect.x, rect.y, rect.width, rect.height);
    CommandUtils::SetScissor(cmd, rect.x, rect.y, rect.width, rect.height);
}

void SwapChainPresenter::TransitionToRenderTarget(CommandList& cmd)
{
    CommandUtils::Transition(cmd, m_renderTargets[m_frameIndex],
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void SwapChainPresenter::TransitionToPresent(CommandList& cmd)
{
    CommandUtils::Transition(cmd, m_renderTargets[m_frameIndex],
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
}

bool SwapChainPresenter::Present(bool vsync)
{
    if (!m_ready)
    {
        Core::Logger::Error("SwapChainPresenter::Present: 그릴 수 없는 상태다 (크기 변경 실패나 장치 손실 뒤)");
        return false;
    }

    if (IsOffscreen())
    {
        m_cmdScheduler.SignalQueue(CommandType::Direct);
        m_frameIndex = (m_frameIndex + 1) % m_frameCount;
        return true;
    }

    UINT syncInterval = vsync ? 1 : 0;
    UINT flags = (!vsync && m_tearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

    HRESULT hr = m_swapChain->Present(syncInterval, flags);
    if (FAILED(hr))
    {
        Core::Logger::Error("SwapChainPresenter::Present: 화면에 내보내지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
            HandleDeviceLost();

        return false;
    }

    m_cmdScheduler.SignalQueue(CommandType::Direct);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    return true;
}

bool SwapChainPresenter::Resize(Device& device, const Core::Size& size)
{
    if (size.width == 0 || size.height == 0)
    {
        Core::Logger::Error("SwapChainPresenter::Resize: 크기가 0이다 ({}x{}). 이전 상태를 유지한다", size.width, size.height);
        return false;
    }

    if (m_ready && m_size == size)
        return true;

    if (!IsOffscreen() && !m_swapChain)
    {
        Core::Logger::Error("SwapChainPresenter::Resize: 스왑 체인이 없다 (장치 손실 뒤에는 다시 만들 수 없다)");
        return false;
    }

    m_ready = false; // 아래에서 하나라도 실패하면 백버퍼가 없는 채로 남으므로 끝까지 성공해야 다시 켠다
    m_cmdScheduler.WaitIdle(); // GPU 작업 끝날 때까지 대기

    for (std::uint32_t i = 0; i < m_frameCount; ++i)
        m_renderTargets[i].Reset(); // 기존 RTV 리소스 해제

    m_depthBuffer.Reset();
    m_readbackBuffer.Reset();

    if (IsOffscreen())
    {
        m_size = size;
        m_frameIndex = 0;
        if (!CreateFrameRTVs(device) || !CreateDepthBuffer(device) || !CreateReadbackBuffer(device))
            return false;

        m_ready = true;
        return true;
    }

    DXGI_SWAP_CHAIN_DESC desc{}; // SwapChain Resize
    if (const HRESULT hr = m_swapChain->GetDesc(&desc); FAILED(hr))
    {
        Core::Logger::Error("SwapChainPresenter::Resize: 스왑 체인 설명을 얻지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }

    UINT flags = desc.Flags;
    if (m_tearing)
        flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    else
        flags &= ~DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    if (const HRESULT hr = m_swapChain->ResizeBuffers(
        m_frameCount,
        size.width,
        size.height,
        DXGI_FORMAT_UNKNOWN,
        flags); FAILED(hr))
    {
        Core::Logger::Error("SwapChainPresenter::Resize: 버퍼 크기를 바꾸지 못했다 ({}x{}, hr={:#010x})",
            size.width, size.height, static_cast<std::uint32_t>(hr));
        return false;
    }

    m_size = size;
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    if (!CreateFrameRTVs(device)) return false;
    if (!CreateDepthBuffer(device)) return false;

    m_ready = true;
    return true;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChainPresenter::GetCurrentRTV() const
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_frameIndex,
        m_rtvDescriptorSize);
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChainPresenter::GetDSV() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

Resource& SwapChainPresenter::GetCurrentBackbuffer() 
{
    return m_renderTargets[m_frameIndex]; 
}

bool SwapChainPresenter::CreateSwapChain(Device& device, ID3D12CommandQueue* queue, const SwapChainDesc& desc)
{
    DXGI_SWAP_CHAIN_DESC1 scDesc{};
    scDesc.BufferCount = m_frameCount;
    scDesc.Width = desc.size.width;
    scDesc.Height = desc.size.height;
    scDesc.Format = RenderFormat::BackBufferFormat;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.SampleDesc.Count = 1;
    scDesc.Flags = desc.allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    auto factory = device.GetFactory();
    if (!factory)
    {
        Core::Logger::Error("SwapChainPresenter::CreateSwapChain: DXGI 팩토리가 없다 (디바이스를 먼저 초기화해야 한다)");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    if (const HRESULT hr = factory->CreateSwapChainForHwnd(
        queue,
        desc.hwnd,
        &scDesc,
        nullptr,
        nullptr,
        &swapChain1); FAILED(hr))
    {
        Core::Logger::Error("SwapChainPresenter::CreateSwapChain: 스왑 체인을 만들지 못했다 ({}x{}, frameCount={}, hr={:#010x})",
            desc.size.width, desc.size.height, m_frameCount, static_cast<std::uint32_t>(hr));
        return false;
    }

    factory->MakeWindowAssociation(desc.hwnd, DXGI_MWA_NO_ALT_ENTER);

    if (const HRESULT hr = swapChain1.As(&m_swapChain); FAILED(hr))
    {
        Core::Logger::Error("SwapChainPresenter::CreateSwapChain: IDXGISwapChain4를 얻지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }

    return true;
}

bool SwapChainPresenter::CreateRTV(Device& device)
{
    m_rtvHeap = device.CreateDescriptorHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        m_frameCount,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    if (!m_rtvHeap)
        return false;

    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    return CreateFrameRTVs(device);
}

bool SwapChainPresenter::CreateDSV(Device& device)
{
    m_dsvHeap = device.CreateDescriptorHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
        1,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

    return static_cast<bool>(m_dsvHeap);
}

bool SwapChainPresenter::CreateFrameRTVs(Device& device)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    for (std::uint32_t i = 0; i < m_frameCount; ++i)
    {
        if (IsOffscreen())
        {
            // sRGB 뷰로 쓰기 위해 타입 없는 포맷으로 만든다. 처음 상태는 스왑체인 백버퍼처럼 Present다.
            const auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_R8G8B8A8_TYPELESS, m_size.width, m_size.height, 1, 1, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
            D3D12_CLEAR_VALUE clear{};
            clear.Format = RenderFormat::BackBufferSRGBView;
            m_renderTargets[i] = device.CreateResource(
                texDesc, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_PRESENT, &clear);
            if (!m_renderTargets[i])
                return false;
        }
        else if (const HRESULT hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTargets[i].GetAddressOf())); FAILED(hr))
        {
            Core::Logger::Error("SwapChainPresenter::CreateFrameRTVs: {}번 백버퍼를 얻지 못했다 (hr={:#010x})", i, static_cast<std::uint32_t>(hr));
            return false;
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = RenderFormat::BackBufferSRGBView;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.Texture2D.PlaneSlice = 0;

        device->CreateRenderTargetView(m_renderTargets[i].Get(), &rtvDesc, handle);
        handle.Offset(1, m_rtvDescriptorSize);
    }

    return true;
}

bool SwapChainPresenter::CreateDepthBuffer(Device& device)
{
    D3D12_RESOURCE_DESC desc =
        CD3DX12_RESOURCE_DESC::Tex2D(
            m_depthFormat,
            m_size.width,
            m_size.height,
            1,
            0,
            1,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    D3D12_CLEAR_VALUE clear{};
    clear.Format = m_depthFormat;
    clear.DepthStencil.Depth = 1.0f;
    clear.DepthStencil.Stencil = 0;

    m_depthBuffer = device.CreateResource(
        desc,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clear);
    if (!m_depthBuffer)
        return false;

    auto dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = m_depthFormat;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc, dsvHandle);

    return true;
}

std::uint32_t SwapChainPresenter::GetReadbackRowPitch() const
{
    return (m_size.width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) / D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
        * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}

bool SwapChainPresenter::CreateReadbackBuffer(Device& device)
{
    const UINT64 size = static_cast<UINT64>(GetReadbackRowPitch()) * m_size.height;
    m_readbackBuffer = device.CreateResource(
        CD3DX12_RESOURCE_DESC::Buffer(size),
        D3D12_HEAP_TYPE_READBACK,
        D3D12_RESOURCE_STATE_COPY_DEST);

    return static_cast<bool>(m_readbackBuffer);
}

void SwapChainPresenter::RecordReadback(CommandList& cmd)
{
    if (!m_ready || !m_readbackBuffer) return; // 오프스크린이 아니거나 그릴 수 없는 상태면 복사할 화면이 없다

    Resource& backBuffer = m_renderTargets[m_frameIndex];
    CommandUtils::Transition(cmd, backBuffer,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_COPY_SOURCE);

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
    footprint.Footprint.Format = RenderFormat::BackBufferFormat;
    footprint.Footprint.Width = m_size.width;
    footprint.Footprint.Height = m_size.height;
    footprint.Footprint.Depth = 1;
    footprint.Footprint.RowPitch = GetReadbackRowPitch();

    const CD3DX12_TEXTURE_COPY_LOCATION src(backBuffer.Get(), 0);
    const CD3DX12_TEXTURE_COPY_LOCATION dst(m_readbackBuffer.Get(), footprint);
    cmd->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    CommandUtils::Transition(cmd, backBuffer,
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        D3D12_RESOURCE_STATE_PRESENT);
}

bool SwapChainPresenter::ReadPixels(std::vector<std::uint8_t>& out)
{
    if (!m_ready || !m_readbackBuffer)
    {
        Core::Logger::Error("SwapChainPresenter::ReadPixels: 읽을 수 있는 화면이 없다 (오프스크린이 아니거나 크기 변경 실패 뒤)");
        return false;
    }

    m_cmdScheduler.WaitIdle(); // 복사가 끝날 때까지 기다린다

    const std::uint32_t rowPitch = GetReadbackRowPitch();
    const std::uint32_t rowBytes = m_size.width * 4;

    std::uint8_t* data = nullptr;
    if (const HRESULT hr = m_readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&data)); FAILED(hr))
    {
        Core::Logger::Error("SwapChainPresenter::ReadPixels: 읽기 버퍼를 매핑하지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }

    out.resize(static_cast<std::size_t>(rowBytes) * m_size.height);
    for (std::uint32_t y = 0; y < m_size.height; ++y)
        std::memcpy(out.data() + static_cast<std::size_t>(y) * rowBytes, data + static_cast<std::size_t>(y) * rowPitch, rowBytes);

    const D3D12_RANGE written{ 0, 0 };
    m_readbackBuffer->Unmap(0, &written);
    return true;
}

void SwapChainPresenter::HandleDeviceLost()
{
    // GPU device lost 발생 시 진입
    // 현재는 최소 cleanup만 수행하고, 실제 재생성 로직은 상위 시스템에서 처리

    // NOTE:
    // - DXGI_ERROR_DEVICE_REMOVED / RESET 대응용
    // - 여기서는 GPU 리소스 release + pointer invalidate까지만 수행
    // - Device + SwapChain 재생성은 DeviceManager 레벨에서 담당

    m_ready = false; // 이후 Render는 프레임을 건너뛰고 Present와 Resize는 Error로 거절한다
    m_cmdScheduler.WaitIdle();

    // GPU 리소스 invalidate
    for (auto& rt : m_renderTargets)
        rt.Reset();
    m_renderTargets.resize(m_frameCount);

    m_depthBuffer.Reset();
    m_readbackBuffer.Reset();
    m_rtvHeap.Reset();
    m_dsvHeap.Reset();
    m_swapChain.Reset();

    // TODO:
    // - HandleDeviceLost 테스트용 fault injection 추가 시
    //   이 함수가 정상적으로 호출되는지 검증 필요
    // 이걸 정확히 하려면 코딩이 많이 필요하고 테스트 환경이 필요해서 일단 이렇게만.
}