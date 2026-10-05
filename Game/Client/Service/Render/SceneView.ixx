export module Client.Render:SceneView;

import std;
import :RenderView;
import Core.Math;
import Client.Render.View;
import Client.Render.Definition;
import Client.Render.Repository;
import Client.Render.ResourceHandles;

constexpr float DegToRad(float deg) noexcept
{
    return deg * std::numbers::pi_v<float> / 180.0f;
}

export class SceneView : public RenderView
{
public:
    virtual ~SceneView() override = default;

    SceneView(
        RepositoryContainer& repositories,
        MaterialHandle defaultMaterial)
        : RenderView{ ViewType::Scene, repositories }
        , m_defaultMaterial{ defaultMaterial }
    {
    }

    virtual bool IsEmpty() const override
    {
        return m_data.draws.IsEmpty();
    }

    void Reset(const SceneViewContext& context)
    {
        m_data.context = context;
        m_data.draws.Clear();
    }

    SceneViewData TakeData()
    {
        return std::move(m_data);
    }

    void DrawEnvironment(EnvironmentHandle hEnv)
    {
        if (!hEnv) return;

        auto& envRepository = m_repositories.Get<EnvironmentRepository>();
        auto envRes = envRepository.GetIfReady(hEnv);
        if (!envRes)
            return;

        m_data.draws.environment = envRes;
    }

    void DrawSurface(
        MeshHandle hM,
        MaterialHandle hMtl,
        const Core::Matrix& world)
    {
        DrawSurfaceInternal(hM, hMtl, std::nullopt, world);
    }

    void DrawWithShaderOverride(
        MeshHandle hM,
        MaterialHandle hMtl,
        ShaderID shaderID,
        const Core::Matrix& world)
    {
        DrawSurfaceInternal(hM, hMtl, shaderID, world);
    }

    void DrawDebugSurface(
        DebugMeshHandle hDM,
        DebugMaterialHandle hDMtl,
        const Core::Matrix& world)
    {
        auto& debugMeshRepository = m_repositories.Get<DebugMeshRepository>();
        auto meshRes = debugMeshRepository.GetIfReady(hDM);
        if (!meshRes)
            return;

        auto& debugMaterialRepository = m_repositories.Get<DebugMaterialRepository>();
        auto materialRes = debugMaterialRepository.GetIfReady(hDMtl);
        if (!materialRes)
            return;

        m_data.draws.debugSurfaces.push_back(DrawDebugSurfaceItem{
            meshRes,
            materialRes,
            world });
    }

private:
    void DrawSurfaceInternal(
        MeshHandle hM,
        MaterialHandle hMtl,
        std::optional<ShaderID> shaderOverride,
        const Core::Matrix& world)
    {
        if (!hMtl)
            hMtl = m_defaultMaterial;

        auto& meshRepository = m_repositories.Get<MeshRepository>();
        auto meshRes = meshRepository.GetIfReady(hM);
        if (!meshRes)
            return;

        auto& materialRepository = m_repositories.Get<MaterialRepository>();
        auto materialRes = materialRepository.GetIfReady(hMtl);
        if (!materialRes)
            return;

        m_data.draws.surfaces.push_back(DrawSurfaceItem{
            meshRes,
            materialRes,
            shaderOverride,
            world
            });
    }

    MaterialHandle m_defaultMaterial;
    SceneViewData m_data;
};