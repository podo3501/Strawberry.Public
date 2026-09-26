export module Client.Asset.Contract:IAssetLoader;

import std;
import Client.Asset.Data;
import :AssetInput;
import :IAssetProvider;
import :IAssetMetaRegistry;

export struct IAssetLoader
{
	virtual ~IAssetLoader() = default;

	virtual bool PreferStream() const { return false; }
	virtual std::shared_ptr<AssetData> Load(AssetInput& source) = 0;
};

// Generic Loader Factory Helper
export template <typename T>
	std::unique_ptr<IAssetLoader> CreateLoader()
{
	static_assert(std::is_base_of_v<IAssetLoader, T>);
	return std::make_unique<T>();
}

// 구체적 에셋 로더 생성 팩토리 함수 선언부
export std::unique_ptr<IAssetLoader> CreateEnvironmentLoader(IAssetProvider* assetProvider);
export std::unique_ptr<IAssetLoader> CreateTextureCubeLoader();
export std::unique_ptr<IAssetLoader> CreateSphericalHarmonicsLoader();

export std::unique_ptr<IAssetLoader> CreateTextureMetaLoader();
export std::unique_ptr<IAssetLoader> CreateImageTextureLoader(IAssetMetaRegistry* metaRegistry);
export std::unique_ptr<IAssetLoader> CreateKtxTextureLoader(IAssetMetaRegistry* metaRegistry);
export std::unique_ptr<IAssetLoader> CreateMeshJsonLoader();
export std::unique_ptr<IAssetLoader> CreateMeshGltfLoader(IAssetProvider* assetProvider);
export std::unique_ptr<IAssetLoader> CreateMaterialJsonLoader(IAssetProvider* assetProvider);
export std::unique_ptr<IAssetLoader> CreateOggStaticLoader();
export std::unique_ptr<IAssetLoader> CreateWavStaticLoader();
export std::unique_ptr<IAssetLoader> CreateStaticSoundTableLoader();
export std::unique_ptr<IAssetLoader> CreateStreamSoundTableLoader();
export std::unique_ptr<IAssetLoader> CreateOggStreamLoader();
export std::unique_ptr<IAssetLoader> CreateHLSLShaderLoader();
export std::unique_ptr<IAssetLoader> CreateBinaryLoader();