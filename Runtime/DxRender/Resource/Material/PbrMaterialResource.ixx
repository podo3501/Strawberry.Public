export module DxRender.Resource:PbrMaterial;

import std;
import :IPendingResource;
import :Texture;
import :Material;
import DxRender.Definition;
import Contract.Render.Backend.Types;
import Contract.Render.Assets;
import Core.TypeHierarchy;

export class PbrMaterialResource final : public Core::TypeNode<PbrMaterialResource, MaterialResource>, public IPendingResource
{
public:
    ~PbrMaterialResource() override = default;

    PbrMaterialResource() :
        TypeNode{
            MaterialType::PBR,
            PipelineLibrary::Get(
                RegistryShader::PBR,
                RasterPreset::Default,
                PrimitiveTopologyType::Triangle)
        }
    {}

    bool IsReady() const noexcept override { return m_ready; }

    // 세 텍스처가 모두 있고 올라가야 한다. 파일이 없는 normal, arm은 Provider가 내장 텍스처로 채워 주고,
    // 준비된 뒤에만 그려지므로 Get*의 참조는 안전하다.
    bool IsDependencyReady() const noexcept override
    {
        return m_albedo && m_normal && m_arm &&
            m_albedo->IsReady() && m_normal->IsReady() && m_arm->IsReady();
    }

    void MarkReady() override { m_ready = true; }

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