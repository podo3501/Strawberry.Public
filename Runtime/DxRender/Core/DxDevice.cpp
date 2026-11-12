module;

#include "d3dx12.h"
#include <dxgi1_6.h>

module DxRender.Core;

import std;
import :DxDevice;
import :DebugHelper;
import Core.Assert;
import Core.Logger;

using Microsoft::WRL::ComPtr;

bool Device::Initialize(const DeviceOptions& options)
{
    DebugHelper::EnableDebugLayer(options.debug); // 디바이스를 만들기 전에 켜야 한다.

    if (!CreateFactory(options.debug.enableDebugLayer)) return false;
    if (!CreateDevice(options.adapter)) return false;

    DebugHelper::SetupInfoQueue(m_device.Get(), options.debug);
    return true;
}

Resource Device::CreateResource(
    const D3D12_RESOURCE_DESC& desc,
    D3D12_HEAP_TYPE heapType,
    D3D12_RESOURCE_STATES state,
    const D3D12_CLEAR_VALUE* clearValue)
{
    CD3DX12_HEAP_PROPERTIES heap(heapType);

    Core::Assert(desc.Width > 0);
    Core::Assert(desc.Height > 0);
    Core::Assert(desc.DepthOrArraySize > 0);

    Resource res;
    auto hr = m_device->CreateCommittedResource(
        &heap,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        state,
        clearValue,
        IID_PPV_ARGS(res.GetAddressOf()));
    if (FAILED(hr))
    {
        Core::Logger::Error("Device::CreateResource: 리소스를 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return {};
    }

    return res;
}

ComPtr<ID3D12DescriptorHeap> Device::CreateDescriptorHeap(
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    UINT numDescriptors,
    D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc{};
    desc.Type = type;
    desc.NumDescriptors = numDescriptors;
    desc.Flags = flags;

    ComPtr<ID3D12DescriptorHeap> heap;
    auto hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf()));
    if (FAILED(hr))
    {
        Core::Logger::Error("Device::CreateDescriptorHeap: 디스크립터 힙을 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return nullptr;
    }

    return heap;
}

ComPtr<ID3D12QueryHeap> Device::CreateQueryHeap(D3D12_QUERY_HEAP_TYPE type, UINT count)
{
    D3D12_QUERY_HEAP_DESC desc{};
    desc.Type = type;
    desc.Count = count;

    ComPtr<ID3D12QueryHeap> heap;
    auto hr = m_device->CreateQueryHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf()));
    if (FAILED(hr))
    {
        Core::Logger::Error("Device::CreateQueryHeap: 쿼리 힙을 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return nullptr;
    }

    return heap;
}

ComPtr<ID3D12Fence> Device::CreateFence(UINT64 initialValue, D3D12_FENCE_FLAGS flags)
{
    ComPtr<ID3D12Fence> fence;
    auto hr = m_device->CreateFence(
        initialValue,
        flags,
        IID_PPV_ARGS(fence.GetAddressOf()));
    if (FAILED(hr))
    {
        Core::Logger::Error("Device::CreateFence: 펜스를 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return nullptr;
    }

    return fence;
}

UINT64 Device::GetRequiredIntermediateSize(
    const D3D12_RESOURCE_DESC& desc,
    UINT firstSubresource,
    UINT numSubresources,
    UINT64 offset)
{
    UINT64 size = 0;

    m_device->GetCopyableFootprints(
        &desc,
        firstSubresource,
        numSubresources,
        offset,
        nullptr,
        nullptr,
        nullptr,
        &size);

    return size;
}


bool Device::CreateFactory(bool enableDebug)
{
    UINT flags = enableDebug ? DXGI_CREATE_FACTORY_DEBUG : 0;
    const HRESULT hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_dxgiFactory));
    if (FAILED(hr))
    {
        Core::Logger::Error("Device::CreateFactory: DXGI 팩토리를 만들지 못했다 (debug={}, hr={:#010x})",
            enableDebug, static_cast<std::uint32_t>(hr));
        return false;
    }

    return true;
}

bool Device::TryCreateDevice(IUnknown* adapter)
{
    const D3D_FEATURE_LEVEL levels[] =
    {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1
    };

    for (auto level : levels)
    {
        if (SUCCEEDED(D3D12CreateDevice(adapter, level, IID_PPV_ARGS(&m_device))))
        {
            m_featureLevel = level;
            return true;
        }
    }

    return false;
}

bool Device::CreateDevice(AdapterKind kind)
{
    if (kind == AdapterKind::Warp)
        return CreateWarpDevice();

    ComPtr<IDXGIAdapter1> adapter;

    for (UINT i = 0; m_dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

        if (TryCreateDevice(adapter.Get()))
            return true;

        adapter.Reset();
    }

    return CreateWarpDevice(); // 쓸 수 있는 하드웨어 어댑터가 없으면 WARP
}

bool Device::CreateWarpDevice()
{
    ComPtr<IDXGIAdapter> warpAdapter;
    const HRESULT hr = m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
    if (FAILED(hr))
    {
        Core::Logger::Error("Device::CreateWarpDevice: WARP 어댑터를 얻지 못했다 (hr={:#010x})",
            static_cast<std::uint32_t>(hr));
        return false;
    }

    if (!TryCreateDevice(warpAdapter.Get()))
    {
        Core::Logger::Error("Device::CreateWarpDevice: WARP 어댑터로 D3D12 디바이스를 만들지 못했다");
        return false;
    }

    return true;
}