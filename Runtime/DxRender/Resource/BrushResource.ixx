module;

#include <d3d12.h>
#include <climits>

export module DxRender.Resource:Brush;

import std;
import :IPendingResource;
import :Texture;
import Core.Math;
import Contract.Render.IResource;

export class BrushResource : public IResource, public IPendingResource
{
public:
    ~BrushResource() override = default;
    BrushResource() = default;

    BrushResource(const BrushResource&) = delete;
    BrushResource& operator=(const BrushResource&) = delete;
    BrushResource(BrushResource&&) noexcept = default;
    BrushResource& operator=(BrushResource&&) noexcept = default;

    bool IsReady() const noexcept override { return m_ready; }
    bool IsDependencyReady() const noexcept override
    {
        // 텍스처 업로드(비동기) 완료 여부
        return m_texture && m_texture->IsReady();
    }
    void MarkReady() override { m_ready = true; }

    // 원본 영역을 텍스처 크기로 나눈 UV. 영역이 없거나 텍스처가 없거나 크기가 0이면 전체 영역(0, 0, 1, 1)
    Core::Vector4 CalcUVTransform(const std::optional<Core::Rect>& source) const
    {
        constexpr Core::Vector4 fullRegion{ 0.0f, 0.0f, 1.0f, 1.0f };
        if (!source || !m_texture)
            return fullRegion;

        const auto& size = m_texture->GetSize();
        if (size.width == 0 || size.height == 0)
            return fullRegion;

        float texW = static_cast<float>(size.width);
        float texH = static_cast<float>(size.height);

        return Core::Vector4(
            source->x / texW,
            source->y / texH,
            (source->x + source->width) / texW,
            (source->y + source->height) / texH
        );
    }

    void SetTexture(std::shared_ptr<TextureResource> res) { m_texture = std::move(res); }
    std::shared_ptr<TextureResource> GetTexture() { return m_texture; }

    // 텍스처가 없으면 UINT_MAX(없음)
    UINT GetTextureIndex() const noexcept { return m_texture ? m_texture->GetHeapIndex() : UINT_MAX; }

private:
    std::shared_ptr<TextureResource> m_texture{ nullptr };

    bool m_ready{ false };
};