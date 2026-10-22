export module Service.Render:SceneRenderer;

import std;
import :SceneView;
import :OverlayView;
import :RenderView;
import Contract.Render.Service;
import Service.Render.Repository;
import Core.Math;
import Contract.Render.Backend.View;

export class SceneRenderer
{
public:
    ~SceneRenderer() = default;
    SceneRenderer() = delete;
    explicit SceneRenderer(RepositoryContainer& repositories);

    SceneView& AcquireView(const SceneViewContext& context);
    OverlayView& AcquireView(const OverlayViewContext& context);

    void SetLight(const DirectionalLightData& light);
    void DrawShadowCaster(MeshHandle hM, const Core::Matrix& world);
    SceneFrameData Flush();

private:
    RepositoryContainer& m_repositories;

    // Default (Built-in)
    MeshHandle m_uiQuad{};
    MaterialHandle m_defaultMaterial;
    BrushHandle m_defaultBrush;

    DirectionalLightData m_pendingLight;
    std::vector<DrawShadowCasterItem> m_shadowCasters;
    std::array<std::unique_ptr<RenderView>, MaxViewCount> m_views;
};