module;

#include <dxgi1_6.h>
#include "d3dx12.h"

export module DxRender.SwapChainPresenter;

import std;
import DxRender.Core;
import DxRender.Command;
import Core.Math;

export struct SwapChainDesc
{
    HWND hwnd{}; // nullptr이면 스왑체인 없이 텍스처에만 그리는 오프스크린 모드
    Core::Size size{};
    bool allowTearing{ false };
    std::uint32_t frameCount{ 2 };
};

export class SwapChainPresenter
{
public:
    ~SwapChainPresenter();
    SwapChainPresenter(CommandScheduler& cmdScheduler);

    // 크기가 0이거나 frameCount가 0이면 Error와 false. 시작할 때 한 번만 부른다.
    bool Initialize(Device& device, const SwapChainDesc& desc);
    void Clear(CommandList& cmd, float r, float g, float b, float a);

    void SetRenderTarget(CommandList& cmd);
    void SetViewport(CommandList& cmd, const std::optional<Core::Rect>& viewport = std::nullopt);

    void TransitionToRenderTarget(CommandList& cmd);
    void TransitionToPresent(CommandList& cmd);

    bool Present(bool vsync); // 그릴 수 없는 상태(IsReady가 false)면 Error와 false
    // 크기 0은 Error와 false에 이전 상태를 유지한다. 실패하면 백버퍼가 없는 상태(IsReady false)로 남고, 다시 Resize하면 복구를 시도한다.
    bool Resize(Device& device, const Core::Size& size);

    // 백버퍼와 깊이 버퍼가 모두 있어 그릴 수 있는 상태인지 (Resize 실패나 장치 손실 뒤에는 false)
    bool IsReady() const noexcept { return m_ready; }
    const Core::Size& GetSize() const noexcept { return m_size; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;
    Resource& GetCurrentBackbuffer();

    // 오프스크린 모드에서 현재 백버퍼를 읽기 버퍼로 복사한다. 렌더가 끝난 뒤 End 전에 호출한다.
    void RecordReadback(CommandList& cmd);
    // GPU 작업을 기다린 뒤 복사된 화면을 행 패딩 없는 RGBA 8비트로 꺼낸다.
    bool ReadPixels(std::vector<std::uint8_t>& out);
    bool IsOffscreen() const noexcept { return m_offscreen; }

private:
    bool CreateSwapChain(Device& device, ID3D12CommandQueue* queue, const SwapChainDesc& desc);
    bool CreateRTV(Device& device);
    bool CreateDSV(Device& device);
    bool CreateFrameRTVs(Device& device);
    bool CreateDepthBuffer(Device& device);
    bool CreateReadbackBuffer(Device& device);
    std::uint32_t GetReadbackRowPitch() const;
    void HandleDeviceLost();

    CommandScheduler& m_cmdScheduler;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    std::vector<Resource> m_renderTargets;

    Resource m_depthBuffer;
    Resource m_readbackBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    DXGI_FORMAT m_depthFormat{ DXGI_FORMAT_D32_FLOAT };

    Core::Size m_size{};
    bool m_tearing{ false };
    bool m_offscreen{ false };
    bool m_ready{ false };
    std::uint32_t m_frameIndex = 0;
    std::uint32_t m_rtvDescriptorSize = 0;
    std::uint32_t m_frameCount{ 2 };
};