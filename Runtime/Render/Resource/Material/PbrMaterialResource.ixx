export module Runtime.Render.Resource:PbrMaterial;

import std;
import :IPendingResource;
import :Texture;
import :Material;
import Runtime.Render.Definition;
import Client.Render.Definition;
import Client.Asset.Data;

export class PbrMaterialResource final : public MaterialResource, public IPendingResource
{
public:
    virtual ~PbrMaterialResource() override = default;

    PbrMaterialResource() :
        MaterialResource{
            MaterialType::PBR,
            PipelineLibrary::Get(
                RegistryShader::PBR,
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

        // normal, arm은 선택
        if (m_normal && !m_normal->IsReady())
            return false;

        if (m_arm && !m_arm->IsReady())
            return false;

        return true;
    }

    virtual void MarkReady() override { m_ready = true; }

    void SetAlbedo(std::shared_ptr<TextureResource> res) { m_albedo = std::move(res); }
    void SetNormal(std::shared_ptr<TextureResource> res) { m_normal = std::move(res); }
    void SetArm(std::shared_ptr<TextureResource> res) { m_arm = std::move(res); }
    void SetSurface(const PbrSurface& surface) { m_surface = surface; }

    const TextureResource& GetAlbedo() const noexcept { return *m_albedo; }
    const TextureResource& GetNormal() const noexcept { return *m_normal; }
    const TextureResource& GetArm() const noexcept { return *m_arm; }
    const PbrSurface& GetSurface() const noexcept { return m_surface; }

private:
    std::shared_ptr<TextureResource> m_albedo;
    std::shared_ptr<TextureResource> m_normal;
    std::shared_ptr<TextureResource> m_arm;
    PbrSurface m_surface;

    bool m_ready{ false };
};