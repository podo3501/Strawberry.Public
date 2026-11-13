export module DxRender.Resource:PhongMaterial;

import std;
import :IPendingResource;
import :Texture;
import :Material;
import DxRender.Definition;
import Contract.Render.Backend.Types;
import Contract.Render.Assets;
import Core.TypeHierarchy;

export class PhongMaterialResource final : public Core::TypeNode<PhongMaterialResource, MaterialResource>, public IPendingResource
{
public:
    ~PhongMaterialResource() override = default;

    PhongMaterialResource() :
        TypeNode{
            MaterialType::Phong,
            PipelineLibrary::Get(
                RegistryShader::Phong,
                RasterPreset::Default,
                PrimitiveTopologyType::Triangle)
        }
    {}

    bool IsReady() const noexcept override { return m_ready; }

    // 두 텍스처가 모두 있고 올라가야 한다. 파일이 없는 normal은 Provider가 내장 텍스처로 채워 주고,
    // 준비된 뒤에만 그려지므로 Get*의 참조는 안전하다.
    bool IsDependencyReady() const noexcept override
    {
        return m_albedo && m_normal && m_albedo->IsReady() && m_normal->IsReady();
    }

    void MarkReady() override { m_ready = true; }

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