module;

#include <d3d12.h>

export module Runtime.Render.Resource:ViewTarget;

import std;
import Core.Math;
import Client.Render.Interfaces;
import Runtime.Render.Core;
import Runtime.Render.RGResourceID;
import Runtime.Render.Graph;
import Runtime.Render.Factory;
import Runtime.Render.Definition;
import Runtime.Render.Helper;

static Resource CreateColorTarget(Device& device, const Core::Size& size)
{
    auto desc = CreateTextureDescriptor(size.width, size.height, RenderFormat::BackBufferFormat);
    desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = RenderFormat::BackBufferFormat;
    clearValue.Color[0] = clearValue.Color[1] = clearValue.Color[2] = clearValue.Color[3] = 0.0f; // PMA 컨벤션: 완전 투명 = RGBA 모두 0

    return device.CreateResource(
        desc,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        &clearValue);
}

static Resource CreateDepthTarget(Device& device, const Core::Size& size)
{
    auto desc = CreateTextureDescriptor(size.width, size.height, DXGI_FORMAT_R32_TYPELESS);
    desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = RenderFormat::DepthFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    return device.CreateResource(
        desc,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue);
}

export class ViewTargetResource : public IResource
{
public:
    virtual ~ViewTargetResource() override
    {
        if (m_descFactory) // pending 처리 후 소멸자 호출 필요
        {
            m_descFactory->FreeRTV(m_colorRTVIndex);
            m_descFactory->FreeDSV(m_depthDSVIndex);
        }

        if (m_idAllocator)
        {
            m_idAllocator->FreeDynamic(m_colorID);
            m_idAllocator->FreeDynamic(m_depthID);
        }
    }

    ViewTargetResource() = default;

    virtual bool IsReady() const noexcept override { return m_ready; }

    bool Initialize(
        Device& device,
        DescriptorFactory& descFactory,
        RGResourceIDAllocator& idAllocator,
        const Core::Size& size)
    {
        m_descFactory = &descFactory;
        m_idAllocator = &idAllocator;
        m_size = size;

        m_color = CreateColorTarget(device, size);
        m_depth = CreateDepthTarget(device, size);
        if (!m_color || !m_depth) return false;

        m_colorID = idAllocator.AllocateDynamic();
        m_depthID = idAllocator.AllocateDynamic();

        m_colorRTVIndex = descFactory.CreateTextureRTV(m_color, RenderFormat::BackBufferFormat);
        m_depthDSVIndex = descFactory.CreateTextureDSV(m_depth, DXGI_FORMAT_D32_FLOAT);
        m_heapIndex = descFactory.CreateTextureSRV(m_color, RenderFormat::BackBufferFormat);

        bool result = m_colorRTVIndex != UINT_MAX && m_depthDSVIndex != UINT_MAX && m_heapIndex != UINT_MAX;

        if (result)
            m_ready = true;

        return result;
    }

    RGResourceID GetColorID() const noexcept { return m_colorID; }
    RGResourceID GetDepthID() const noexcept { return m_depthID; }
    std::uint32_t GetColorRTVIndex() const noexcept { return m_colorRTVIndex; }
    std::uint32_t GetDepthDSVIndex() const noexcept { return m_depthDSVIndex; }
    std::uint32_t GetHeapIndex() const noexcept { return m_heapIndex; }
    const Core::Size& GetSize() const noexcept { return m_size; }
    const Resource& GetColorResource() const noexcept { return m_color; }
    const Resource& GetDepthResource() const noexcept { return m_depth; }

private:
    DescriptorFactory* m_descFactory{ nullptr };
    RGResourceIDAllocator* m_idAllocator{ nullptr };
    bool m_ready{ false };

    Resource m_color;
    Resource m_depth;
    RGResourceID m_colorID;
    RGResourceID m_depthID;
    std::uint32_t m_colorRTVIndex{ UINT_MAX };
    std::uint32_t m_depthDSVIndex{ UINT_MAX };
    std::uint32_t m_heapIndex{ UINT_MAX }; // bindless index
    Core::Size m_size;
};