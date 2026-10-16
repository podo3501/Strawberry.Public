module;

#include "d3dx12.h"

export module DxRender.Factory:Descriptor;

import DxRender.Core;
import DxRender.Allocator;
import Contract.Render.Types;

export class DescriptorFactory
{
public:
    ~DescriptorFactory() = default;
    DescriptorFactory() = delete;
    explicit DescriptorFactory(Device& device);
    bool Initialize(const DescriptorConfig& config);

    void BeginFrame(UINT slot);

    UINT CreateBufferSRV(
        DescriptorAllocationType type,
        const Resource& resBuffer,
        UINT firstElement,
        UINT elementCount,
        UINT elementStride);

    UINT CreateTextureSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevels = 1);
    UINT CreateTextureRTV(const Resource& res, DXGI_FORMAT format, UINT mipSlice = 0);
    UINT CreateTextureDSV(const Resource& res, DXGI_FORMAT format, UINT mipSlice = 0);
    
    UINT CreateMipSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevel);
    UINT CreateMipUAV(const Resource& res, DXGI_FORMAT format, UINT mipLevel);
    UINT CreateTextureCubeSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevels);

    void FreeRTV(UINT rtvIndex) { m_rtvAllocator.Free(rtvIndex); }
    void FreeDSV(UINT dsvIndex) { m_dsvAllocator.Free(dsvIndex); }
        
    std::uint32_t GetCurrentSlot() const noexcept { return m_currentSlot; }
    BindlessDescriptorAllocator& GetBindlessAllocator() noexcept { return m_bindlessAllocator; }
    DescriptorAllocator& GetDSVAllocator() noexcept { return m_dsvAllocator; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(UINT rtvIndex) { return m_rtvAllocator.GetCpuHandle(rtvIndex); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle(UINT index) { return m_dsvAllocator.GetCpuHandle(index); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetBindlessCpuHandle(UINT index) { return m_bindlessAllocator.GetCpuHandle(index); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetBindlessGpuHandle(UINT index) { return m_bindlessAllocator.GetGpuHandle(index); }

private:
    D3D12_SHADER_RESOURCE_VIEW_DESC CreateStructuredBufferSRVDesc(
        UINT firstElement,
        UINT numElements,
        UINT stride) const;

    Device& m_device;
    std::uint32_t m_currentSlot{ UINT_MAX };
    BindlessDescriptorAllocator m_bindlessAllocator;
    DescriptorAllocator m_rtvAllocator;
    DescriptorAllocator m_dsvAllocator;
};