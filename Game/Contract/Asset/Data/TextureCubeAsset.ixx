export module Client.Asset.Data:TextureCubeAsset;

import std;
import :TextureTypes;
import Client.Asset.AssetData;
import Core.TypeHierarchy;

export struct TextureCubeMipFace
{
    std::uint32_t width{ 0 };
    std::uint32_t height{ 0 };
    std::uint32_t rowPitch{ 0 }; // byte
    std::vector<std::uint8_t> pixels;
};

export struct TextureCubeAsset : public Core::TypeNode<TextureCubeAsset, AssetData>
{
    virtual ~TextureCubeAsset() = default;

    PixelFormat format{ PixelFormat::Unknown };
    ColorSpace colorSpace{ ColorSpace::Linear }; // HDR 환경맵은 보통 리니어

    uint32_t width{ 0 }; // mip0 기준
    uint32_t height{ 0 };
    uint32_t mipCount{ 0 };
    uint32_t faceCount{ 6 };

    std::vector<TextureCubeMipFace> subImages; // index = mip + face * mipCount  (D3D12 subresource 인덱싱 규칙과 동일하게 맞춰둠 -> 나중에 업로드할 때 편함)
};