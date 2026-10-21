export module Service.Asset:AssetRepository;

import std;
import :AssetCacheKeys;
import :AssetLoaderDesc;
import Core.ResourceID;
import Contract.Asset.Service;
import Contract.Asset.Backend;
import Contract.Asset.Shared;
import Contract.Common.Asset;

export class AssetRepository : public IAssetProvider
{
public:
	~AssetRepository() override = default;
	explicit AssetRepository(IAssetStorage* assetStorage) noexcept;

	bool RegisterLoader(AssetLoaderDesc desc);
	std::shared_ptr<Asset> Load(Core::TypeID type, const Core::ResourceID& resID) override;

private:
	std::shared_ptr<Asset> LoadWithSource(IAssetLoader* loader, const Core::ResourceID& resID);

	IAssetStorage* m_assetStorage{ nullptr };
	std::unordered_map<LoaderKey, std::unique_ptr<IAssetLoader>, LoaderKeyHasher> m_loaders;

	std::unordered_map<CacheKey, std::weak_ptr<Asset>, CacheKeyHasher> m_cache;
	mutable std::mutex m_cacheMutex;
};