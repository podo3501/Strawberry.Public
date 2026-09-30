module;

#include <d3d12.h>

export module Runtime.Render.Resource:Shadow;

import std;
import Core.Math;
import Runtime.Render.Core;
import Runtime.Render.Definition;
import Runtime.Render.Factory;
import Runtime.Render.Helper;

constexpr Core::Size ShadowMapSize = { 2048, 2048 };

static Resource CreateShadowResource(Device& device)
{
    auto desc = CreateTextureDescriptor(ShadowMapSize.width, ShadowMapSize.height, DXGI_FORMAT_R32_TYPELESS);
    desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = RenderFormat::ShadowMapFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    return device.CreateResource(
        desc,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue);
}

export class ShadowResource
{
public:
    ~ShadowResource() = default;
    ShadowResource() = default;

    ShadowResource(const ShadowResource&) = delete;
    ShadowResource& operator=(const ShadowResource&) = delete;
    ShadowResource(ShadowResource&&) noexcept = default;
    ShadowResource& operator=(ShadowResource&&) noexcept = default;

    bool Initialize(Device& device, DescriptorFactory& factory)
    {
        m_resource = CreateShadowResource(device);
        if (!m_resource)
            return false;

        m_dsvIndex = factory.CreateTextureDSV(m_resource, DXGI_FORMAT_D32_FLOAT);
        m_srvIndex = factory.CreateTextureSRV(m_resource, DXGI_FORMAT_R32_FLOAT);

        if (m_dsvIndex == UINT_MAX || m_srvIndex == UINT_MAX)
            return false;

        return true;
    }

    const Resource& GetResource() const noexcept { return m_resource; }
    Resource& GetResource() noexcept { return m_resource; }

    UINT GetDSVIndex() const noexcept { return m_dsvIndex; } // 쓰기용 인덱스
    UINT GetSRVIndex() const noexcept { return m_srvIndex; } // 읽기용 Bindless 인덱스

private:
    Resource m_resource{};
    UINT m_dsvIndex{ UINT_MAX };
    UINT m_srvIndex{ UINT_MAX };
};