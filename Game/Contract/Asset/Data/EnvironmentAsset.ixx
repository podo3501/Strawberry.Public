export module Client.Asset.Data:EnvironmentAsset;

import std;
import :TextureCubeAsset;
import :SphericalHarmonicsAsset;
import Client.Asset.AssetData;
import Core.TypeHierarchy;

export struct EnvironmentAsset : public Core::TypeNode<EnvironmentAsset, AssetData>
{
    virtual ~EnvironmentAsset() = default;

    std::shared_ptr<TextureCubeAsset> skybox;
    std::shared_ptr<TextureCubeAsset> reflection;
    std::shared_ptr<SphericalHarmonicsAsset> irradiance;
};
