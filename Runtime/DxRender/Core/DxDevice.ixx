module;

#include "d3dx12.h"
#include <dxgi1_6.h>

export module DxRender.Core:DxDevice;

import :DxResource;
import :DebugOptions;

using Microsoft::WRL::ComPtr;

export enum class AdapterKind
{
    Auto, // 하드웨어 어댑터를 먼저 시도하고 없으면 WARP
    Warp  // WARP로 고정 (결과가 기계마다 같다)
};

export struct DeviceOptions
{
    DebugOptions debug{ .enableDebugLayer = false };
    AdapterKind adapter{ AdapterKind::Auto };
};

export class Device
{
public:
    ~Device() = default;
    Device() = default;

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    bool Initialize(const DeviceOptions& options); // 시작할 때 한 번만 부른다.

    Resource CreateResource(
        const D3D12_RESOURCE_DESC& desc,
        D3D12_HEAP_TYPE heapType,
        D3D12_RESOURCE_STATES state,
        const D3D12_CLEAR_VALUE* clearValue = nullptr);

    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        UINT numDescriptors,
        D3D12_DESCRIPTOR_HEAP_FLAGS flags);

    ComPtr<ID3D12QueryHeap> CreateQueryHeap(D3D12_QUERY_HEAP_TYPE type, UINT count);
    ComPtr<ID3D12Fence> CreateFence(UINT64 initialValue, D3D12_FENCE_FLAGS flags);

    UINT64 GetRequiredIntermediateSize(
        const D3D12_RESOURCE_DESC& desc,
        UINT firstSubresource,
        UINT numSubresources,
        UINT64 offset);

    ID3D12Device* operator->() const { return m_device.Get(); }
    ID3D12Device* Get() { return m_device.Get(); }
    IDXGIFactory4* GetFactory() const { return m_dxgiFactory.Get(); }
    D3D_FEATURE_LEVEL GetFeatureLevel() const { return m_featureLevel; }

private:
    bool CreateFactory(bool enableDebug);
    bool TryCreateDevice(IUnknown* adapter);
    bool CreateDevice(AdapterKind kind);
    bool CreateWarpDevice();

private:
    Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;

    D3D_FEATURE_LEVEL m_featureLevel{ D3D_FEATURE_LEVEL_11_0 };
};