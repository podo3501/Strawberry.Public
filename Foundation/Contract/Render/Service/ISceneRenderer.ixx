export module Contract.Render.Service:IFrameRecorder;

import :ISceneView;
import Contract.Render.Handles;
import Contract.Render.IOverlayView;
import Contract.Render.Shared.View;
import Core.Math;

export struct IFrameRecorder
{
	virtual ~IFrameRecorder() = default;

	virtual ISceneView& AcquireView(const SceneViewContext& context) = 0;
	virtual IOverlayView& AcquireView(const OverlayViewContext& context) = 0;

	virtual void SetLight(const DirectionalLightData& light) = 0;
	virtual void DrawShadowCaster(MeshHandle hM, const Core::Matrix& world) = 0;
};
