export module Contract.Asset.Data:TextureMetaAsset;

import std;
import Contract.Asset.AssetData;
import :TextureTypes;
import Core.TypeHierarchy;

export enum class AlphaSourceState
{
    Opaque,               // 알파를 색상 투명도로 안 씀 (변환 불필요)
    Straight,             // 원본이 non-premultiplied. 로드 시 곱해줘야 함     
    AlreadyPremultiplied, // 원본이 이미 premultiplied로 export됨. 그대로 사용, 곱하면 안 됨
    Count,
};

export enum class BlendTargetSpace
{
    NonPremultiplied, // straight alpha 블렌드 패스에서 소비됨
    Premultiplied,    // PMA 블렌드 패스에서 소비됨
    None,             // 알파 블렌딩 자체를 안 함 (opaque 렌더)
    Count,
};

export struct TextureMetaAsset : public Core::TypeNode<TextureMetaAsset, AssetData>
{
    virtual ~TextureMetaAsset() = default;

    ColorSpace colorSpace{ ColorSpace::SRGB };
    bool generateMipmaps{ false };
    AlphaSourceState alphaSourceState{ AlphaSourceState::Straight };
    BlendTargetSpace blendTargetSpace{ BlendTargetSpace::NonPremultiplied };
};
