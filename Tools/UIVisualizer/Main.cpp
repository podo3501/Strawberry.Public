// ImGui 단독 실행 확인용. ImGui 공식 example_win32_directx12를 바탕으로 한다.
// 창, 스왑체인, 커맨드 큐는 이 파일이 직접 만들고 DxRender는 아직 쓰지 않는다.
#include <Imgui/imgui.h>
#include <Imgui/imgui_impl_win32.h>
#include <Imgui/imgui_impl_dx12.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <objbase.h>
#include <cassert>

#ifdef _DEBUG
#include <dxgidebug.h>
#pragma comment(lib, "Imgui_Debug.lib")
#pragma comment(lib, "dxguid.lib")
#else
#pragma comment(lib, "Imgui_Release.lib")
#endif
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

import std;
import UIVisualizer.UiPreview;
import UIVisualizer.PanelInput;
import UIVisualizer.WidgetTreePanel;
import UIVisualizer.PropertyPanel;
import Contract.Render.Shared;
import Core.Logger;
import Core.Math;

namespace
{
    struct FrameContext
    {
        ID3D12CommandAllocator* commandAllocator{};
        UINT64 fenceValue{};
    };

    constexpr int kFramesInFlight = 3;
    constexpr int kBackBuffers = 3;

    FrameContext g_frameContext[kFramesInFlight]{};
    UINT g_frameIndex{};

    ID3D12Device* g_device{};
    ID3D12DescriptorHeap* g_rtvHeap{};
    ID3D12DescriptorHeap* g_srvHeap{};
    ID3D12CommandQueue* g_commandQueue{};
    ID3D12GraphicsCommandList* g_commandList{};
    ID3D12Fence* g_fence{};
    HANDLE g_fenceEvent{};
    UINT64 g_fenceLastSignaledValue{};
    IDXGISwapChain3* g_swapChain{};
    bool g_swapChainOccluded{};
    HANDLE g_swapChainWaitable{};
    ID3D12Resource* g_backBuffer[kBackBuffers]{};
    D3D12_CPU_DESCRIPTOR_HANDLE g_backBufferRtv[kBackBuffers]{};

    // 엔진 UI를 그린 결과를 ImGui가 보여주기 위한 텍스처. SRV 힙의 0번은 ImGui 폰트가 쓴다.
    constexpr UINT kPreviewSrvIndex = 1;
    ID3D12Resource* g_previewTexture{};
    ID3D12Resource* g_previewUpload[kFramesInFlight]{};
    D3D12_GPU_DESCRIPTOR_HANDLE g_previewSrvGpu{};
    Core::Size g_previewSize{};
    FrameImage g_previewImage;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void WaitForLastSubmittedFrame()
{
    FrameContext& frame = g_frameContext[g_frameIndex % kFramesInFlight];
    const UINT64 fenceValue = frame.fenceValue;
    if (fenceValue == 0) return; // 신호를 보낸 적이 없다

    frame.fenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue) return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

static FrameContext* WaitForNextFrameResources()
{
    const UINT nextFrameIndex = ++g_frameIndex;

    HANDLE waitables[] = { g_swapChainWaitable, nullptr };
    DWORD waitableCount = 1;

    FrameContext* frame = &g_frameContext[nextFrameIndex % kFramesInFlight];
    if (frame->fenceValue != 0)
    {
        const UINT64 fenceValue = frame->fenceValue;
        frame->fenceValue = 0;
        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        waitables[1] = g_fenceEvent;
        waitableCount = 2;
    }

    WaitForMultipleObjects(waitableCount, waitables, TRUE, INFINITE);
    return frame;
}

static void WaitForAllFrames()
{
    if (g_fenceLastSignaledValue == 0 || g_fence->GetCompletedValue() >= g_fenceLastSignaledValue) return;

    g_fence->SetEventOnCompletion(g_fenceLastSignaledValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

static UINT PreviewRowPitch(UINT width)
{
    return (width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) / D3D12_TEXTURE_DATA_PITCH_ALIGNMENT * D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}

static void ReleasePreviewTexture()
{
    WaitForAllFrames(); // 이전 프레임들이 텍스처를 다 쓸 때까지 기다린다
    if (g_previewTexture) { g_previewTexture->Release(); g_previewTexture = nullptr; }
    for (auto& upload : g_previewUpload)
        if (upload) { upload->Release(); upload = nullptr; }
}

static bool CreatePreviewTexture(const Core::Size& size)
{
    ReleasePreviewTexture();

    D3D12_HEAP_PROPERTIES heap{};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = size.width;
    desc.Height = size.height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    if (FAILED(g_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&g_previewTexture))))
        return false;

    const UINT srvSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE cpu = g_srvHeap->GetCPUDescriptorHandleForHeapStart();
    cpu.ptr += static_cast<SIZE_T>(srvSize) * kPreviewSrvIndex;
    g_previewSrvGpu = g_srvHeap->GetGPUDescriptorHandleForHeapStart();
    g_previewSrvGpu.ptr += static_cast<UINT64>(srvSize) * kPreviewSrvIndex;

    D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
    srv.Format = desc.Format;
    srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv.Texture2D.MipLevels = 1;
    g_device->CreateShaderResourceView(g_previewTexture, &srv, cpu);

    D3D12_HEAP_PROPERTIES uploadHeap{};
    uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC bufferDesc{};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = static_cast<UINT64>(PreviewRowPitch(size.width)) * size.height;
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    for (auto& upload : g_previewUpload)
        if (FAILED(g_device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload))))
            return false;

    g_previewSize = size;
    return true;
}

// 프레임 슬롯의 업로드 버퍼에 읽어 온 화면을 쓰고 텍스처로 복사하는 명령을 기록한다
static void RecordPreviewUpload(UINT frameSlot)
{
    if (!g_previewTexture || g_previewImage.pixels.empty() || g_previewImage.size != g_previewSize) return;

    const UINT width = g_previewSize.width;
    const UINT height = g_previewSize.height;
    const UINT rowPitch = PreviewRowPitch(width);

    std::uint8_t* mapped = nullptr;
    if (FAILED(g_previewUpload[frameSlot]->Map(0, nullptr, reinterpret_cast<void**>(&mapped)))) return;
    for (UINT y = 0; y < height; ++y)
        std::memcpy(mapped + static_cast<std::size_t>(y) * rowPitch, g_previewImage.pixels.data() + static_cast<std::size_t>(y) * width * 4, static_cast<std::size_t>(width) * 4);
    g_previewUpload[frameSlot]->Unmap(0, nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = g_previewTexture;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    g_commandList->ResourceBarrier(1, &barrier);

    D3D12_TEXTURE_COPY_LOCATION dst{};
    dst.pResource = g_previewTexture;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION src{};
    src.pResource = g_previewUpload[frameSlot];
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    src.PlacedFootprint.Footprint.Width = width;
    src.PlacedFootprint.Footprint.Height = height;
    src.PlacedFootprint.Footprint.Depth = 1;
    src.PlacedFootprint.Footprint.RowPitch = rowPitch;
    g_commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    g_commandList->ResourceBarrier(1, &barrier);
}

static void CreateRenderTargets()
{
    for (UINT i = 0; i < kBackBuffers; ++i)
    {
        g_swapChain->GetBuffer(i, IID_PPV_ARGS(&g_backBuffer[i]));
        g_device->CreateRenderTargetView(g_backBuffer[i], nullptr, g_backBufferRtv[i]);
    }
}

static void CleanupRenderTargets()
{
    WaitForLastSubmittedFrame();

    for (auto& buffer : g_backBuffer)
        if (buffer) { buffer->Release(); buffer = nullptr; }
}

static bool CreateDeviceD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.BufferCount = kBackBuffers;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sd.Scaling = DXGI_SCALING_STRETCH;

#ifdef _DEBUG
    ID3D12Debug* debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
        debug->EnableDebugLayer();
#endif

    if (D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_device)) != S_OK)
        return false;

#ifdef _DEBUG
    if (debug)
    {
        ID3D12InfoQueue* infoQueue = nullptr;
        if (SUCCEEDED(g_device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
        {
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            infoQueue->Release();
        }
        debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = kBackBuffers;
        desc.NodeMask = 1;
        if (g_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_rtvHeap)) != S_OK)
            return false;

        const SIZE_T rtvSize = g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE handle = g_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (auto& rtv : g_backBufferRtv)
        {
            rtv = handle;
            handle.ptr += rtvSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 2; // 0: ImGui 폰트, 1: 엔진 UI 미리보기
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (g_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_srvHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.NodeMask = 1;
        if (g_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_commandQueue)) != S_OK)
            return false;
    }

    for (auto& frame : g_frameContext)
        if (g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.commandAllocator)) != S_OK)
            return false;

    if (g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].commandAllocator, nullptr, IID_PPV_ARGS(&g_commandList)) != S_OK ||
        g_commandList->Close() != S_OK)
        return false;

    if (g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!g_fenceEvent)
        return false;

    {
        IDXGIFactory4* factory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&factory)) != S_OK)
            return false;
        if (factory->CreateSwapChainForHwnd(g_commandQueue, hwnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
        {
            factory->Release();
            return false;
        }
        const HRESULT hr = swapChain1->QueryInterface(IID_PPV_ARGS(&g_swapChain));
        swapChain1->Release();
        factory->Release();
        if (hr != S_OK)
            return false;

        g_swapChain->SetMaximumFrameLatency(kBackBuffers);
        g_swapChainWaitable = g_swapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTargets();
    return true;
}

static void CleanupDeviceD3D()
{
    ReleasePreviewTexture();
    CleanupRenderTargets();
    if (g_swapChain) { g_swapChain->SetFullscreenState(false, nullptr); g_swapChain->Release(); g_swapChain = nullptr; }
    if (g_swapChainWaitable) { CloseHandle(g_swapChainWaitable); g_swapChainWaitable = nullptr; }
    for (auto& frame : g_frameContext)
        if (frame.commandAllocator) { frame.commandAllocator->Release(); frame.commandAllocator = nullptr; }
    if (g_commandQueue) { g_commandQueue->Release(); g_commandQueue = nullptr; }
    if (g_commandList) { g_commandList->Release(); g_commandList = nullptr; }
    if (g_rtvHeap) { g_rtvHeap->Release(); g_rtvHeap = nullptr; }
    if (g_srvHeap) { g_srvHeap->Release(); g_srvHeap = nullptr; }
    if (g_fence) { g_fence->Release(); g_fence = nullptr; }
    if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
    if (g_device) { g_device->Release(); g_device = nullptr; }

#ifdef _DEBUG
    IDXGIDebug1* dxgiDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
    {
        dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        dxgiDebug->Release();
    }
#endif
}

static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_device && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTargets();
            const HRESULT hr = g_swapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam),
                DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
            assert(SUCCEEDED(hr) && "Failed to resize swapchain.");
            (void)hr;
            CreateRenderTargets();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Alt 키로 메뉴가 열리지 않게 한다
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

namespace
{
    // COM 초기화와 정리. UiPreview보다 먼저 만들어서 서비스가 모두 해제된 뒤에 CoUninitialize가 불리게 한다
    class ComScope
    {
    public:
        ComScope() : m_initialized(SUCCEEDED(CoInitialize(nullptr))) {}
        ~ComScope() { if (m_initialized) CoUninitialize(); }
        ComScope(const ComScope&) = delete;
        ComScope& operator=(const ComScope&) = delete;

    private:
        bool m_initialized;
    };
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"StrawberryUIVisualizer", nullptr };
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Strawberry UIVisualizer", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    Core::Logger::Initialize();   // UiPreview가 서비스를 만들기 전에 로거와 COM이 준비되어 있어야 한다
    ComScope com;
    UiPreview preview;
    PanelInputReader panelInput;
    WidgetTreePanel widgetTree;
    PropertyPanel propertyPanel;
    if (!preview.Initialize({ 640, 480 }))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(g_device, kFramesInFlight, DXGI_FORMAT_R8G8B8A8_UNORM, g_srvHeap,
        g_srvHeap->GetCPUDescriptorHandleForHeapStart(),
        g_srvHeap->GetGPUDescriptorHandleForHeapStart());

    const ImVec4 clearColor(0.45f, 0.55f, 0.60f, 1.00f);

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // 화면이 가려졌거나 잠겨 있으면 그리지 않고 쉰다
        if (g_swapChainOccluded && g_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            Sleep(10);
            continue;
        }
        g_swapChainOccluded = false;

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::SetNextWindowPos(ImVec2(720.f, 60.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(320.f, 220.f), ImGuiCond_FirstUseEver);
        widgetTree.Draw(preview.Ui());

        ImGui::SetNextWindowPos(ImVec2(720.f, 300.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(320.f, 260.f), ImGuiCond_FirstUseEver);
        propertyPanel.Draw(widgetTree.Selected());

        ImGui::SetNextWindowSize(ImVec2(640.f, 480.f), ImGuiCond_FirstUseEver); // 크기를 안 주면 안의 이미지에 맞춰 계속 줄어든다
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::Begin("Preview", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();
        {
            // 패널 크기에 맞춰 엔진이 UI를 그리고, 결과를 텍스처로 올려 보여준다
            const ImVec2 avail = ImGui::GetContentRegionAvail();
            const Core::Size panelSize{ static_cast<std::uint32_t>(std::max(avail.x, 1.f)), static_cast<std::uint32_t>(std::max(avail.y, 1.f)) };
            preview.Resize(panelSize);

            // 보이지 않는 버튼을 패널 전체에 깔아 마우스를 받는다 (창이 끌려 움직이지 않게 한다)
            const ImVec2 origin = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("##PreviewInput", ImVec2(static_cast<float>(panelSize.width), static_cast<float>(panelSize.height)));
            const bool hovered = ImGui::IsItemHovered();
            preview.ApplyInput(panelInput.Read(origin.x, origin.y, hovered, ImGui::IsWindowFocused()));
            ImGui::SetCursorScreenPos(origin);

            if (preview.Frame(io.DeltaTime, g_previewImage) && g_previewImage.size != g_previewSize)
                CreatePreviewTexture(g_previewImage.size);

            if (g_previewTexture)
                ImGui::Image(static_cast<ImTextureID>(g_previewSrvGpu.ptr),
                    ImVec2(static_cast<float>(g_previewSize.width), static_cast<float>(g_previewSize.height)));

            widgetTree.DrawSelectionOutline(origin.x, origin.y);
        }
        ImGui::End();

        ImGui::Render();

        FrameContext* frame = WaitForNextFrameResources();
        const UINT backBufferIndex = g_swapChain->GetCurrentBackBufferIndex();
        frame->commandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = g_backBuffer[backBufferIndex];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_commandList->Reset(frame->commandAllocator, nullptr);
        RecordPreviewUpload(static_cast<UINT>(frame - g_frameContext));
        g_commandList->ResourceBarrier(1, &barrier);

        const float clear[4] = { clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w };
        g_commandList->ClearRenderTargetView(g_backBufferRtv[backBufferIndex], clear, 0, nullptr);
        g_commandList->OMSetRenderTargets(1, &g_backBufferRtv[backBufferIndex], FALSE, nullptr);
        g_commandList->SetDescriptorHeaps(1, &g_srvHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_commandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_commandList->ResourceBarrier(1, &barrier);
        g_commandList->Close();

        g_commandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&g_commandList));

        const HRESULT hr = g_swapChain->Present(1, 0); // 수직 동기화
        g_swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

        const UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        g_commandQueue->Signal(g_fence, fenceValue);
        g_fenceLastSignaledValue = fenceValue;
        frame->fenceValue = fenceValue;
    }

    WaitForLastSubmittedFrame();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}
