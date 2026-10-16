module;

#include "d3dx12.h"

module DxRender.Factory;

import :Descriptor;
import Core.Assert;

DescriptorFactory::DescriptorFactory(Device& device)
    : m_device{ device }
{}

bool DescriptorFactory::Initialize(const DescriptorConfig& config)
{
    if (!m_bindlessAllocator.Initialize(m_device)) return false;
    if (!m_rtvAllocator.Initialize(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, config.rtvCount)) return false;
    if (!m_dsvAllocator.Initialize(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, config.dsvCount)) return false;

    return true;
}

void DescriptorFactory::BeginFrame(UINT slot)
{
    m_currentSlot = slot;
    m_bindlessAllocator.ResetTransient(slot);
}

UINT DescriptorFactory::CreateBufferSRV(
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

UINT DescriptorFactory::CreateTextureSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevels)
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

UINT DescriptorFactory::CreateTextureRTV(const Resource& res, DXGI_FORMAT format, UINT mipSlice)
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

UINT DescriptorFactory::CreateTextureDSV(const Resource& res, DXGI_FORMAT format, UINT mipSlice)
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

UINT DescriptorFactory::CreateMipSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevel)
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

UINT DescriptorFactory::CreateMipUAV(const Resource& res, DXGI_FORMAT format, UINT mipLevel)
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

UINT DescriptorFactory::CreateTextureCubeSRV(const Resource& res, DXGI_FORMAT format, UINT mipLevels)
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

D3D12_SHADER_RESOURCE_VIEW_DESC DescriptorFactory::CreateStructuredBufferSRVDesc(
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