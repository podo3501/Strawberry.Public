export module Contract.Asset.Data:TextureAsset;

import std;
import :TextureTypes;
import Contract.Asset.AssetData;
import Core.TypeHierarchy;
import Core.Math;

export struct TextureAsset : public Core::TypeNode<TextureAsset, AssetData>
{
    virtual ~TextureAsset() = default;

    Core::Size size;
    std::uint32_t stride{ 0 };
    PixelFormat format{ PixelFormat::RGBA8 };
    std::vector<std::uint8_t> pixels{};

    ColorSpace colorSpace{ ColorSpace::SRGB };
    bool generateMipmaps{ false };
    bool isPremultipliedAlpha{ false };
};
