export module DxRender.Provider:BuiltinTexture;

import std;

export using TextureSlot = std::uint32_t;

export enum class BuiltinTextureType
{
    White,       // 일반 컬러/알베도용 (1,1,1,1)
    FlatNormal,  // 노멀 맵용 (0.5, 0.5, 1.0)
    DefaultARM,  // ARM용 (1, 0.5, 0)
    Count
};

export struct BuiltinTextureBinding
{
    TextureSlot slot;
    BuiltinTextureType type;
};