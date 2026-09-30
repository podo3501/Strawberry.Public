export module Client.Render.Definition:ShaderTypes;

import std;

export using ShaderID = std::uint32_t;
export inline constexpr ShaderID InvalidShaderID{ 0 };

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