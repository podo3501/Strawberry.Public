export module Service.Render:FrameRecorder;

import std;
import :SceneView;
import :OverlayView;
import :RenderView;
import Service.Render.Repository;
import Contract.Render.Service;
import Contract.Render.Backend.View;
import Core.Math;

export class FrameRecorder : public IFrameRecorder
{
public:
    ~FrameRecorder() override = default;
    FrameRecorder() = delete;
    explicit FrameRecorder(RepositoryContainer& repositories);

	ISceneView& AcquireView(const SceneViewContext& context) override;
	IOverlayView& AcquireView(const OverlayViewContext& context) override;

	void SetLight(const DirectionalLightData& light) override;
	void DrawShadowCaster(MeshHandle hM, const Core::Matrix& world) override;

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