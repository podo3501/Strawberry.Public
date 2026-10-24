export module Contract.Render.Shared.Types:ShaderID;

import std;
export using ShaderID = std::uint32_t;
export inline constexpr ShaderID InvalidShaderID{ 0 };