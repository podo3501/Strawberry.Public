module;

#include <d3d12.h>

export module DxRender.Resource:Shadow;

import std;
import DxRender.Core;
import Core.Logger;

export class ShadowResource
{
public:
    ~ShadowResource() = default;
    ShadowResource() = default;

    ShadowResource(const ShadowResource&) = delete;
    ShadowResource& operator=(const ShadowResource&) = delete;
    ShadowResource(ShadowResource&&) noexcept = default;
    ShadowResource& operator=(ShadowResource&&) noexcept = default;

    // 서술자 인덱스가 없음(UINT_MAX)이면 Error를 남기고 상태를 바꾸지 않는다.
    bool Initialize(Resource resource, UINT dsvIndex, UINT srvIndex)
    {
        if (dsvIndex == UINT_MAX || srvIndex == UINT_MAX)
        {
            Core::Logger::Error("ShadowResource::Initialize: 서술자 인덱스가 올바르지 않다 (dsv={}, srv={})", dsvIndex, srvIndex);
            return false;
        }

        if (resource)
        {
            const D3D12_RESOURCE_DESC desc = resource->GetDesc();
            m_width = static_cast<UINT>(desc.Width);
            m_height = desc.Height;
        }
        m_resource = std::move(resource);
        m_dsvIndex = dsvIndex;
        m_srvIndex = srvIndex;

        return true;
    }

    const Resource& GetResource() const noexcept { return m_resource; }
    Resource& GetResource() noexcept { return m_resource; }

    UINT GetWidth() const noexcept { return m_width; }   // 텍스처 크기. 그림자 패스의 뷰포트가 이 값을 쓴다
    UINT GetHeight() const noexcept { return m_height; }
    UINT GetDSVIndex() const noexcept { return m_dsvIndex; } // 쓰기용 인덱스
    UINT GetSRVIndex() const noexcept { return m_srvIndex; } // 읽기용 Bindless 인덱스

private:
    Resource m_resource{};
    UINT m_width{ 0 };
    UINT m_height{ 0 };
    UINT m_dsvIndex{ UINT_MAX };
    UINT m_srvIndex{ UINT_MAX };
};