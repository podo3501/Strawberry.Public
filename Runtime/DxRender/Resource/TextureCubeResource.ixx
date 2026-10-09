module;

#include "d3dx12.h"

export module DxRender.Resource:TextureCube;

import std;
import DxRender.Core;
import Core.Assert;
import Core.Math;
import Contract.Asset.Data;

export struct TextureCubeDesc
{
    ColorSpace colorSpace{ ColorSpace::Linear }; // 큐브맵은 항상 Linear
};

export class TextureCubeResource
{
public:
    ~TextureCubeResource() = default;
    TextureCubeResource() = default;

    TextureCubeResource(const TextureCubeResource&) = delete;
    TextureCubeResource& operator=(const TextureCubeResource&) = delete;
    TextureCubeResource(TextureCubeResource&&) noexcept = default;
    TextureCubeResource& operator=(TextureCubeResource&&) noexcept = default;

    bool IsReady() const noexcept { return m_ready; }
    void MarkReady() noexcept { m_ready = true; }

    void SetDesc(const TextureCubeDesc& desc) noexcept { m_desc = desc; }
    const TextureCubeDesc& GetDesc() const noexcept { return m_desc; }

    const Resource& Get() const noexcept { return m_texture; }
    Resource& Get() noexcept { return m_texture; }
    void Set(Resource resource) noexcept { m_texture = std::move(resource); }

    void SetSize(const Core::Size& size) noexcept { m_size = size; }
    const Core::Size& GetSize() const noexcept { return m_size; }

    void SetHeapIndex(UINT index) noexcept { m_heapIndex = index; }
    UINT GetHeapIndex() const noexcept { return m_heapIndex; }

    UINT GetMipCount() const noexcept
    {
        Core::Assert(m_texture); // IsReady() 이후에만 호출해야 함
        return m_texture->GetDesc().MipLevels;
    }

private:
    TextureCubeDesc m_desc{};
    Resource m_texture{};

    Core::Size m_size{};
    UINT m_heapIndex{ UINT_MAX };
    bool m_ready{ false };
};