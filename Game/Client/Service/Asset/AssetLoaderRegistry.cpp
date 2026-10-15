module Service.Asset;

import :AssetLoaderRegistry;
import Service.Asset.ImageExtensions;
import Core.Utils;

AssetLoaderRegistry::AssetLoaderRegistry(AssetRepository& repository) noexcept
    : m_repository{ repository }
{}

bool AssetLoaderRegistry::RegisterDefaultLoaders(IAssetMetaRegistry* metaRegistry)
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