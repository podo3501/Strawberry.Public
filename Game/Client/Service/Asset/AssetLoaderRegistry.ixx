export module Client.Asset.Service:AssetLoaderRegistry;

import std;
import :AssetRepository;
import Core.Utils;
import Client.Asset.Contract;
import Client.AssetMetaRegistry;
import Client.Asset.ImageExtensions;

export class AssetLoaderRegistry
{
public:
    ~AssetLoaderRegistry() = default;

    explicit AssetLoaderRegistry(AssetRepository& repository) noexcept
        : m_repository{ repository }
    {
    }

    bool RegisterDefaultLoaders(IAssetMetaRegistry* metaRegistry)
    {
        bool ok = true;

        // Meta
        ok &= RegisterLoader<TextureMetaAsset>(".meta", CreateTextureMetaLoader());

        // Asset
        ok &= RegisterLoader<BinaryAsset>(".bin", CreateBinaryLoader());
        ok &= RegisterLoader<BinaryAsset>(".ttf", CreateBinaryLoader());
        ok &= RegisterLoader<EnvironmentAsset>(".envmap", CreateEnvironmentLoader(&m_repository));
        ok &= RegisterLoader<TextureCubeAsset>(".ktx2", CreateTextureCubeLoader());
        ok &= RegisterLoader<SphericalHarmonicsAsset>(".txt", CreateSphericalHarmonicsLoader());

        for (auto& ext : ImageSupportedExtensions)
            ok &= RegisterLoader<TextureAsset>(ext, CreateImageTextureLoader(metaRegistry));

        ok &= RegisterLoader<MeshAsset>(".mjson", CreateMeshJsonLoader());
        ok &= RegisterLoader<MeshAsset>(".gltf", CreateMeshGltfLoader(&m_repository));
        ok &= RegisterLoader<PbrMaterialAsset>(".material", CreateMaterialJsonLoader(&m_repository));
        ok &= RegisterLoader<PhongMaterialAsset>(".material", CreateMaterialJsonLoader(&m_repository));
        ok &= RegisterLoader<StaticSoundTable>(".Json", CreateStaticSoundTableLoader());
        ok &= RegisterLoader<StreamSoundTable>(".Json", CreateStreamSoundTableLoader());
        ok &= RegisterLoader<StaticSoundAsset>(".ogg", CreateOggStaticLoader());
        ok &= RegisterLoader<StreamSoundAsset>(".ogg", CreateOggStreamLoader());
        ok &= RegisterLoader<StaticSoundAsset>(".wav", CreateWavStaticLoader());
        ok &= RegisterLoader<ShaderAsset>(".hlsl", CreateHLSLShaderLoader());

        return ok;
    }

private:
    template <typename AssetType>
    bool RegisterLoader(std::string_view ext, std::unique_ptr<IAssetLoader>&& loader)
    {
        return m_repository.RegisterLoader(
            AssetLoaderDesc::Make<AssetType>(ext, std::move(loader))
        );
    }

private:
    AssetRepository& m_repository;
};