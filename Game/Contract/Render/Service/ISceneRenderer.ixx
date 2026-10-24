export module Contract.Render.Service:ISceneRenderer;

import :Handles;
import :ISceneView;
import :IOverlayView;
import Contract.Render.Shared.View;
import Core.Math;

export struct ISceneRenderer
{
	virtual ~ISceneRenderer() = default;

	virtual ISceneView& AcquireView(const SceneViewContext& context) = 0;
	virtual IOverlayView& AcquireView(const OverlayViewContext& context) = 0;

	virtual void SetLight(const DirectionalLightData& light) = 0;
	virtual void DrawShadowCaster(MeshHandle hM, const Core::Matrix& world) = 0;
};
