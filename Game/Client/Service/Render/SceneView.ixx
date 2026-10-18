export module Service.Render:SceneView;

import std;
import :RenderView;
import Core.Math;
import Contract.Render.View;
import Contract.Render.Types;
import Service.Render.Repository;
import Service.Render.Handles;

export class SceneView : public RenderView
{
public:
    virtual ~SceneView() override = default;
    SceneView() = delete;
    SceneView(
        RepositoryContainer& repositories,
        MaterialHandle defaultMaterial);
    virtual bool IsEmpty() const override;

    void Reset(const SceneViewContext& context);
    SceneViewData TakeData();

    void DrawEnvironment(EnvironmentHandle hEnv);

    void DrawSurface(
        MeshHandle hM,
        MaterialHandle hMtl,
        const Core::Matrix& world);

    void DrawWithShaderOverride(
        MeshHandle hM,
        MaterialHandle hMtl,
        ShaderID shaderID,
        const Core::Matrix& world);

    void DrawDebugSurface(
        DebugMeshHandle hDM,
        DebugMaterialHandle hDMtl,
        const Core::Matrix& world);

private:
    void DrawSurfaceInternal(
        MeshHandle hM,
        MaterialHandle hMtl,
        std::optional<ShaderID> shaderOverride,
        const Core::Matrix& world);

    MaterialHandle m_defaultMaterial;
    SceneViewData m_data;
};