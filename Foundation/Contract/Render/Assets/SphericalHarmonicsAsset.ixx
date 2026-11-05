export module Contract.Render.Assets:SphericalHarmonicsAsset;

import std;
import Contract.Common.Asset;
import Core.TypeHierarchy;
import Core.Math;

export struct SphericalHarmonicsAsset : public Core::TypeNode<SphericalHarmonicsAsset, Asset>
{
    virtual ~SphericalHarmonicsAsset() = default;

    std::array<Core::Vector3, 9> coefficients;
};
