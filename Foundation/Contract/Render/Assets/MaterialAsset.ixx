export module Contract.Render.Assets:MaterialAsset;

import std;
import :MaterialTypes;
import :TextureAsset;
import Contract.Common.Asset;
import Core.TypeHierarchy;

export struct MaterialAsset : public Core::TypeNode<MaterialAsset, Asset>
{
    virtual ~MaterialAsset() = default;

    MaterialType type{ MaterialType::Phong };

    std::shared_ptr<TextureAsset> albedo{ nullptr };
    std::shared_ptr<TextureAsset> normal{ nullptr };
};
