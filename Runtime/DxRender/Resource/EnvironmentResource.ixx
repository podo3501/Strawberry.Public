export module DxRender.Resource:Environment;

import std;
import Core.Math;
import Contract.Render.IResource;
import :IPendingResource;
import :TextureCube;

export class EnvironmentResource : public IResource, public IPendingResource
{
public:
    virtual ~EnvironmentResource() override = default;
    EnvironmentResource() = default;

    EnvironmentResource(const EnvironmentResource&) = delete;
    EnvironmentResource& operator=(const EnvironmentResource&) = delete;
    EnvironmentResource(EnvironmentResource&&) noexcept = default;
    EnvironmentResource& operator=(EnvironmentResource&&) noexcept = default;

    virtual bool IsReady() const noexcept override { return m_ready; }
    virtual bool IsDependencyReady() const noexcept override
    {
        // 텍스처 업로드(비동기) 완료 여부. SH는 CPU 데이터 복사라 즉시 완료됨.
        return m_skybox && m_reflection && m_skybox->IsReady() && m_reflection->IsReady();
    }
    virtual void MarkReady() override { m_ready = true; }

    void SetSkybox(std::shared_ptr<TextureCubeResource> res) { m_skybox = std::move(res); }
    void SetReflection(std::shared_ptr<TextureCubeResource> res) { m_reflection = std::move(res); }
    void SetIrradianceSH(const std::array<Core::Vector3, 9>& sh) { m_irradianceSH = sh; }

    std::shared_ptr<TextureCubeResource> GetSkybox() const { return m_skybox; }
    std::shared_ptr<TextureCubeResource> GetReflection() const { return m_reflection; }
    std::array<Core::Vector3, 9> GetIrradianceSH() const { return m_irradianceSH; }

private:
    std::shared_ptr<TextureCubeResource> m_skybox;
    std::shared_ptr<TextureCubeResource> m_reflection;
    std::array<Core::Vector3, 9> m_irradianceSH{};

    bool m_ready{ false };
};