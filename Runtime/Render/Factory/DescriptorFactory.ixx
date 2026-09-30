module;

#include "d3dx12.h"

export module Runtime.Render.Factory:Descriptor;

import std;
import Core.Assert;
import Runtime.Render.Core;
import Runtime.Render.Allocator;
import Client.Render.Definition;

export class DescriptorFactory
{
public:
    ~DescriptorFactory() = default;
    DescriptorFactory() = delete;
    explicit DescriptorFactory(Device& device)
        : m_device{ device }
    {
    }

    bool Initialize(const DescriptorConfig& config)
    {
        if (!m_bindlessAllocator.Initialize(m_device)) return false;
        if (!m_rtvAllocator.Initialize(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, config.rtvCount)) return false;
        if (!m_dsvAllocator.Initialize(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, config.dsvCount)) return false;

        return true;
    }

    void BeginFrame(UINT slot)
    {
        m_currentSlot = slot;
        m_bindlessAllocator.ResetTransient(slot);
    }

    UINT CreateBufferSRV(
        DescriptorAllocationType type,
        const Resource& resBuffer,
        UINT firstElement,
        UINT elementCount,
        UINT elementStride)
    {
        UINT index = UINT_MAX;
        switch (type)
        {
        case DescriptorAllocationType::Persistent: index = m_bindlessAllocator.AllocatePersistent(); break;
        case DescriptorAllocationType::Transient: index = m_bindlessAllocator.AllocateTransient(m_currentSlot); break;
        case DescriptorAllocationType::Dynamic: Core::Assert(false); break; // 지원 안 함
        }
        if (index == UINT_MAX)
            return UINT_MAX;

        auto srvDesc = CreateStructuredBufferSRVDesc(firstElement, elementCount, elementStride);
        m_device->CreateShaderResourceView(resBuffer.Get(), &srvDesc, GetBindlessCpuHandle(index));
        return index;
    }

    UINT CreateTextureSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevels = 1)
    {
        const auto& resDesc = res->GetDesc();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = mipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

        UINT index = m_bindlessAllocator.AllocatePersistent();
        if (index == UINT_MAX) return UINT_MAX;

        m_device->CreateShaderResourceView(res.Get(), &srvDesc, GetBindlessCpuHandle(index));
        return index;
    }

    UINT CreateTextureRTV(const Resource& res, DXGI_FORMAT format, UINT mipSlice = 0)
    {
        if (!res) return UINT_MAX;

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = format;
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = mipSlice;
        rtvDesc.Texture2D.PlaneSlice = 0;

        UINT index = m_rtvAllocator.Allocate();
        if (index == UINT_MAX) return UINT_MAX;

        m_device->CreateRenderTargetView(res.Get(), &rtvDesc, GetRTVHandle(index));
        return index;
    }

    UINT CreateTextureDSV(const Resource& res, DXGI_FORMAT format, UINT mipSlice = 0)
    {
        if (!res) return UINT_MAX;

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = format;
        dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = mipSlice; // 기본값 0, 필요시 특정 밉슬라이스 지정 가능
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        // DSV 전용 할당자(m_dsvAllocator)에서 공간 확보
        UINT index = m_dsvAllocator.Allocate();
        if (index == UINT_MAX) return UINT_MAX;

        m_device->CreateDepthStencilView(res.Get(), &dsvDesc, GetDSVHandle(index));
        return index;
    }

    //이 함수는 DescriptorFactory에 존재하면 안된다. 이 함수를 호출하는 시점에서 이 함수를 복사해서 그 클래스의 private 멤버 함수로 넣어놓자.
    //bool CreateTextureViews(
    //    TextureResource* texRes,
    //    bool generateMips,
    //    std::vector<UINT>* outMipSrvIndices = nullptr,
    //    std::vector<UINT>* outMipUavIndices = nullptr)
    //{
    //    if (!texRes) return false;

    //    auto& res = texRes->Get();
    //    const auto& resDesc = res->GetDesc();
    //    const UINT mipCount = resDesc.MipLevels;

    //    DXGI_FORMAT srvFormat = resDesc.Format;
    //    if (texRes->GetDesc().colorSpace == ColorSpace::SRGB)
    //        srvFormat = ToSRGB(resDesc.Format);

    //    UINT mainMipLevels = generateMips ? mipCount : 1;
    //    UINT mainIndex = CreateTextureSRV(res, srvFormat, mainMipLevels);
    //    if (mainIndex == UINT_MAX) return false;

    //    texRes->SetHeapIndex(mainIndex);

    //    if (generateMips && mipCount > 1)
    //    {
    //        if (outMipSrvIndices) outMipSrvIndices->reserve(mipCount);
    //        if (outMipUavIndices) outMipUavIndices->reserve(mipCount);

    //        for (UINT i = 0; i < mipCount; ++i)
    //        {
    //            UINT mipSrvIndex = CreateMipSRV(res, srvFormat, i);
    //            if (mipSrvIndex == UINT_MAX) return false;
    //            if (outMipSrvIndices) outMipSrvIndices->push_back(mipSrvIndex);

    //            UINT mipUavIndex = CreateMipUAV(res, resDesc.Format, i);
    //            if (mipUavIndex == UINT_MAX) return false;
    //            if (outMipUavIndices) outMipUavIndices->push_back(mipUavIndex);
    //        }
    //    }

    //    return true;
    //}

    //이 함수는 DescriptorFactory에 존재하면 안된다. 이 함수를 호출하는 시점에서 이 함수를 복사해서 그 클래스의 private 멤버 함수로 넣어놓자.
    //bool CreateTextureCubeViews(TextureCubeResource* texRes)
    //{
    //    if (!texRes) return false;

    //    auto& res = texRes->Get();
    //    const auto& resDesc = res->GetDesc();
    //    const UINT mipCount = resDesc.MipLevels;

    //    DXGI_FORMAT srvFormat = resDesc.Format; // 큐브맵은 항상 Linear -> sRGB 변환 불필요
    //    UINT index = CreateTextureCubeSRV(res, srvFormat, mipCount);
    //    if (index == UINT_MAX) return false;

    //    texRes->SetHeapIndex(index);
    //    return true;
    //}

    void FreeRTV(UINT rtvIndex)
    {
        m_rtvAllocator.Free(rtvIndex);
    }

    void FreeDSV(UINT dsvIndex)
    {
        m_dsvAllocator.Free(dsvIndex);
    }

    std::uint32_t GetCurrentSlot() const noexcept { return m_currentSlot; }
    BindlessDescriptorAllocator& GetBindlessAllocator() noexcept { return m_bindlessAllocator; }
    DescriptorAllocator& GetDSVAllocator() noexcept { return m_dsvAllocator; }

    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(UINT rtvIndex)
    {
        return m_rtvAllocator.GetCpuHandle(rtvIndex);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle(UINT index)
    {
        return m_dsvAllocator.GetCpuHandle(index);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetBindlessCpuHandle(UINT index)
    {
        return m_bindlessAllocator.GetCpuHandle(index);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetBindlessGpuHandle(UINT index)
    {
        return m_bindlessAllocator.GetGpuHandle(index);
    }

private:
    D3D12_SHADER_RESOURCE_VIEW_DESC CreateStructuredBufferSRVDesc(
        UINT firstElement,
        UINT numElements,
        UINT stride) const
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC desc{};

        desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer로 인식시키기 위해 Format은 UNKNOWN으로 설정

        desc.Buffer.FirstElement = firstElement;
        desc.Buffer.NumElements = numElements;
        desc.Buffer.StructureByteStride = stride; // 구조체(Vertex 등)의 크기
        desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        return desc;
    }

    UINT CreateMipSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevel)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = mipLevel;
        srvDesc.Texture2D.MipLevels = 1;

        UINT index = m_bindlessAllocator.AllocateDynamic();
        if (index == UINT_MAX) return UINT_MAX;

        m_device->CreateShaderResourceView(res.Get(), &srvDesc, GetBindlessCpuHandle(index));
        return index;
    }

    UINT CreateMipUAV(const Resource& res, DXGI_FORMAT format, UINT mipLevel)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.Format = format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = mipLevel;

        UINT index = m_bindlessAllocator.AllocateDynamic();
        if (index == UINT_MAX) return UINT_MAX;

        m_device->CreateUnorderedAccessView(res.Get(), nullptr, &uavDesc, GetBindlessCpuHandle(index));
        return index;
    }

    UINT CreateTextureCubeSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevels)
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = mipLevels;
        srvDesc.TextureCube.MostDetailedMip = 0;
        srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

        UINT index = m_bindlessAllocator.AllocatePersistent();
        if (index == UINT_MAX) return UINT_MAX;

        m_device->CreateShaderResourceView(res.Get(), &srvDesc, GetBindlessCpuHandle(index));
        return index;
    }

    Device& m_device;
    std::uint32_t m_currentSlot{ UINT_MAX };
    BindlessDescriptorAllocator m_bindlessAllocator;
    DescriptorAllocator m_rtvAllocator;
    DescriptorAllocator m_dsvAllocator;
};