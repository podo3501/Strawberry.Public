export module Contract.Asset.Data:PbrMaterialAsset;

import std;
import :PbrSurface;
import :MaterialAsset;
import :TextureAsset;
import Core.TypeHierarchy;

export struct PbrMaterialAsset : public Core::TypeNode<PbrMaterialAsset, MaterialAsset>
{
    virtual ~PbrMaterialAsset() = default;

    PbrSurface surface;
    std::shared_ptr<TextureAsset> arm{ nullptr };
};
