export module Contract.Render.Shared.Types:ShaderTypes;

export enum class ShaderType
{
	Graphics,
	Compute
};

export enum class RasterPreset
{
	Default,
	NoCull,
	Wireframe,
	WireframeNoCull
};