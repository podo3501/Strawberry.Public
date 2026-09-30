module;

#include "d3dx12.h"

export module Runtime.Render.Resource:Texture;

import std;
import Runtime.Render.Core;
import Core.Math;
import Client.Asset.Data;

export struct TextureDesc
{
    ColorSpace colorSpace{ ColorSpace::SRGB };
    bool generateMipmaps{ false };
    bool isPremultiplyAlpha{ false };
};

export class TextureResource
{
public:
    ~TextureResource() = default;
    TextureResource() = default;

    TextureResource(const TextureResource&) = delete;
    TextureResource& operator=(const TextureResource&) = delete;
    TextureResource(TextureResource&&) noexcept = default;
    TextureResource& operator=(TextureResource&&) noexcept = default;

    bool IsReady() const noexcept { return m_ready; }
    void MarkReady() noexcept { m_ready = true; }

    void SetPremultiplyAlpha(bool premultiplyAlpha) noexcept { m_premultiplyAlpha = premultiplyAlpha; }

    void SetDesc(const TextureDesc& desc) noexcept { m_desc = desc; }
    const TextureDesc& GetDesc() const noexcept { return m_desc; }

    const Resource& Get() const noexcept { return m_texture; }
    Resource& Get() noexcept { return m_texture; }
    void Set(Resource resource) noexcept { m_texture = std::move(resource); }

    void SetSize(const Core::Size& size) noexcept { m_size = size; }
    const Core::Size& GetSize() const noexcept { return m_size; }

    void SetHeapIndex(UINT index) noexcept { m_heapIndex = index; }
    UINT GetHeapIndex() const noexcept { return m_heapIndex; }

private:
    TextureDesc m_desc{};
    bool m_premultiplyAlpha{ false };
    Resource m_texture{};

    Core::Size m_size{};
    UINT m_heapIndex{ UINT_MAX };
    bool m_ready{ false };
};