export module Client.Asset.Data:PhongMaterialAsset;

import std;
import :MaterialAsset;
import :PhongSurface;
import Core.TypeHierarchy;

export struct PhongMaterialAsset : public Core::TypeNode<PhongMaterialAsset, MaterialAsset>
{
    virtual ~PhongMaterialAsset() = default;

    PhongSurface surface;
};
