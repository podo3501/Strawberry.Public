export module Client.Asset.Data:MaterialAsset;

import std;
import :MaterialTypes;
import :TextureAsset;
import Client.Asset.AssetData;
import Core.TypeHierarchy;

export struct MaterialAsset : public Core::TypeNode<MaterialAsset, AssetData>
{
    virtual ~MaterialAsset() = default;

    MaterialType type{ MaterialType::Phong };

    std::shared_ptr<TextureAsset> albedo{ nullptr };
    std::shared_ptr<TextureAsset> normal{ nullptr };
};
