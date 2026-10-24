export module Contract.Render.Service:ISceneView;

import :Handles;
import Contract.Render.Shared.Types;
import Core.Math;

export struct ISceneView
{
	virtual ~ISceneView() = default;

	virtual void DrawEnvironment(EnvironmentHandle hEnv) = 0;

	virtual void DrawSurface(
		MeshHandle hM,
		MaterialHandle hMtl,
		const Core::Matrix& world) = 0;

	virtual void DrawWithShaderOverride(
		MeshHandle hM,
		MaterialHandle hMtl,
		ShaderID shaderID,
		const Core::Matrix& world) = 0;

	virtual void DrawDebugSurface(
		DebugMeshHandle hDM,
		DebugMaterialHandle hDMtl,
		const Core::Matrix& world) = 0;
};