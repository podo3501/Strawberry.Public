module;

#include "d3dx12.h"
#include <dxgi1_6.h>

export module DxRender.Core:DxDevice;

import std;
import :DebugHelper;
import :DebugOptions;
import :DxResource;
import Core.Assert;

using Microsoft::WRL::ComPtr;

export class Device
{
public:
    ~Device() = default;
    explicit Device(bool enableDebug)
    {
#if defined(_DEBUG)
        DebugOptions opt;
        opt.enableDebugLayer = true;
        opt.enableGpuValidation = false; // 필요할 때만 true. 계속 켜 놓으면 오래 걸린다.
        opt.breakOnWarning = false;     // Warning도 잡는다.

        if (enableDebug)
            DebugHelper::EnableDebugLayer(opt);
#endif

        bool result = CreateFactory(enableDebug);
        Core::Assert(result);

        result = CreateDevice();
        Core::Assert(result);

#if defined(_DEBUG)
        DebugHelper::SetupInfoQueue(m_device.Get(), opt);
#endif
    }

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Resource CreateResource(
        const D3D12_RESOURCE_DESC& desc,
        D3D12_HEAP_TYPE heapType,
        D3D12_RESOURCE_STATES state,
        const D3D12_CLEAR_VALUE* clearValue = nullptr)
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
        Core::Assert(SUCCEEDED(hr));

        return res;
    }

    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
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
        Core::Assert(SUCCEEDED(hr));

        return heap;
    }

    ComPtr<ID3D12QueryHeap> CreateQueryHeap(D3D12_QUERY_HEAP_TYPE type, UINT count)
    {
        D3D12_QUERY_HEAP_DESC desc{};
        desc.Type = type;
        desc.Count = count;

        ComPtr<ID3D12QueryHeap> heap;
        auto hr = m_device->CreateQueryHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf()));
        Core::Assert(SUCCEEDED(hr));

        return heap;
    }

    ComPtr<ID3D12Fence> CreateFence(UINT64 initialValue, D3D12_FENCE_FLAGS flags)
    {
        ComPtr<ID3D12Fence> fence;
        auto hr = m_device->CreateFence(
            initialValue,
            flags,
            IID_PPV_ARGS(fence.GetAddressOf()));
        Core::Assert(SUCCEEDED(hr));

        return fence;
    }

    UINT64 GetRequiredIntermediateSize(
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

    ID3D12Device* operator->() const { return m_device.Get(); }
    ID3D12Device* Get() { return m_device.Get(); }
    IDXGIFactory4* GetFactory() const { return m_dxgiFactory.Get(); }
    D3D_FEATURE_LEVEL GetFeatureLevel() const { return m_featureLevel; }

private:
    bool CreateFactory(bool enableDebug)
    {
        UINT flags = enableDebug ? DXGI_CREATE_FACTORY_DEBUG : 0;
        return SUCCEEDED(CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_dxgiFactory)));
    }

    bool TryCreateDevice(IUnknown* adapter)
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

    bool CreateDevice()
    {
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

        ComPtr<IDXGIAdapter> warpAdapter; // fallback
        if (FAILED(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter))))
            return false;

        return TryCreateDevice(warpAdapter.Get());
    }

private:
    Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;

    D3D_FEATURE_LEVEL m_featureLevel{ D3D_FEATURE_LEVEL_11_0 };
};