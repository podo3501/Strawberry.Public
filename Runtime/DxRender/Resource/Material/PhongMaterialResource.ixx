export module DxRender.Resource:PhongMaterial;

import std;
import :IPendingResource;
import :Texture;
import :Material;
import DxRender.Definition;
import Contract.Render.Definition;
import Contract.Asset.Data;

export class PhongMaterialResource final : public MaterialResource, public IPendingResource
{
public:
    virtual ~PhongMaterialResource() override = default;

    PhongMaterialResource() :
        MaterialResource{
            MaterialType::Phong,
            PipelineLibrary::Get(
                RegistryShader::Phong,
                RasterPreset::Default,
                PrimitiveTopologyType::Triangle)
        }
    {}

    virtual bool IsReady() const noexcept override { return m_ready; }

    virtual bool IsDependencyReady() const noexcept override
    {
        // albedo는 필수
        if (!m_albedo || !m_albedo->IsReady())
            return false;

        // normal은 선택
        if (m_normal && !m_normal->IsReady())
            return false;

        return true;
    }

    virtual void MarkReady() override { m_ready = true; }

    void SetAlbedo(std::shared_ptr<TextureResource> res) { m_albedo = std::move(res); }
    void SetNormal(std::shared_ptr<TextureResource> res) { m_normal = std::move(res); }
    void SetSurface(const PhongSurface& surface) { m_surface = surface; }

    const TextureResource& GetAlbedo() const noexcept { return *m_albedo; }
    const TextureResource& GetNormal() const noexcept { return *m_normal; }
    const PhongSurface& GetSurface() const noexcept { return m_surface; }

private:
    std::shared_ptr<TextureResource> m_albedo;
    std::shared_ptr<TextureResource> m_normal;
    PhongSurface m_surface;

    bool m_ready{ false };
};