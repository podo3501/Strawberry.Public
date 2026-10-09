export module Contract.Asset.Data:SphericalHarmonicsAsset;

import std;
import Contract.Asset.AssetData;
import Core.TypeHierarchy;
import Core.Math;

export struct SphericalHarmonicsAsset : public Core::TypeNode<SphericalHarmonicsAsset, AssetData>
{
    virtual ~SphericalHarmonicsAsset() = default;

    std::array<Core::Vector3, 9> coefficients;
};
