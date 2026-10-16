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
    HWND hwnd{};
    Core::Size size{};
    bool allowTearing{ false };
    std::uint32_t frameCount{ 2 };
};

export class SwapChainPresenter
{
public:
    ~SwapChainPresenter();
    SwapChainPresenter(CommandScheduler& cmdScheduler);

    bool Initialize(Device& device, const SwapChainDesc& desc);
    void Clear(CommandList& cmd, float r, float g, float b, float a);

    void SetRenderTarget(CommandList& cmd);
    void SetViewport(CommandList& cmd, const std::optional<Core::Rect>& viewport = std::nullopt);

    void TransitionToRenderTarget(CommandList& cmd);
    void TransitionToPresent(CommandList& cmd);

    bool Present(bool vsync);
    bool Resize(Device& device, const Core::Size& size);

    const Core::Size& GetSize() const noexcept { return m_size; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRTV() const;
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const;
    Resource& GetCurrentBackbuffer();

private:
    bool CreateSwapChain(Device& device, ID3D12CommandQueue* queue, const SwapChainDesc& desc);
    bool CreateRTV(Device& device);
    bool CreateDSV(Device& device);
    bool CreateFrameRTVs(Device& device);
    bool CreateDepthBuffer(Device& device);
    void HandleDeviceLost();

    CommandScheduler& m_cmdScheduler;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    std::vector<Resource> m_renderTargets;

    Resource m_depthBuffer;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    DXGI_FORMAT m_depthFormat{ DXGI_FORMAT_D32_FLOAT };

    Core::Size m_size{};
    bool m_tearing{ false };
    std::uint32_t m_frameIndex = 0;
    std::uint32_t m_rtvDescriptorSize = 0;
    std::uint32_t m_frameCount{ 2 };
};