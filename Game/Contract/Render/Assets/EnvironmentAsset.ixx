export module Contract.Render.Assets:EnvironmentAsset;

import std;
import :TextureCubeAsset;
import :SphericalHarmonicsAsset;
import Contract.Common.Asset;
import Core.TypeHierarchy;

export struct EnvironmentAsset : public Core::TypeNode<EnvironmentAsset, Asset>
{
    virtual ~EnvironmentAsset() = default;

    std::shared_ptr<TextureCubeAsset> skybox;
    std::shared_ptr<TextureCubeAsset> reflection;
    std::shared_ptr<SphericalHarmonicsAsset> irradiance;
};
