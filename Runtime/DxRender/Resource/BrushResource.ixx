export module DxRender.Resource:Brush;

import std;
import :IPendingResource;
import :Texture;
import Core.Assert;
import Core.Math;
import Contract.Render.IResource;

export class BrushResource : public IResource, public IPendingResource
{
public:
    virtual ~BrushResource() override = default;
    BrushResource() = default;

    BrushResource(const BrushResource&) = delete;
    BrushResource& operator=(const BrushResource&) = delete;
    BrushResource(BrushResource&&) noexcept = default;
    BrushResource& operator=(BrushResource&&) noexcept = default;

    virtual bool IsReady() const noexcept override { return m_ready; }
    virtual bool IsDependencyReady() const noexcept override
    {
        // 텍스처 업로드(비동기) 완료 여부. SH는 CPU 데이터 복사라 즉시 완료됨.
        return m_texture && m_texture->IsReady();
    }
    virtual void MarkReady() override { m_ready = true; }

    Core::Vector4 CalcUVTransform(const std::optional<Core::Rect>& source) const
    {
        if (!source)
            return Core::Vector4(0.0f, 0.0f, 1.0f, 1.0f);

        const auto& size = m_texture->GetSize();

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

    UINT GetTextureIndex() const noexcept
    {
        Core::Assert(m_texture); // default texture 라도 들고 있어야 함
        return m_texture->GetHeapIndex();
    }

private:
    std::shared_ptr<TextureResource> m_texture{ nullptr };
    Core::Vector4 m_uvTransform{ 0.0f, 0.0f, 1.0f, 1.0f };

    bool m_ready{ false };
};