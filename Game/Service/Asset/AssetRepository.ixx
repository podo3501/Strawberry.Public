export module Service.Asset:AssetRepository;

import std;
import :AssetCacheKeys;
import :AssetLoaderDesc;
import Core.AssetPath;
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
	std::shared_ptr<Asset> LoadWithSource(IAssetLoader* loader, const Core::ResourceID& resID, const Core::AssetPath& path);
	std::shared_ptr<Asset> LoadUncached(Core::TypeID type, const Core::ResourceID& resID, const Core::AssetPath& path);
	void SweepExpiredCache(); // m_cacheMutex를 잡은 채로 부른다. 해제된 에셋의 빈 항목을 치운다

	IAssetStorage* m_assetStorage{ nullptr };
	std::unordered_map<LoaderKey, std::unique_ptr<IAssetLoader>, LoaderKeyHasher> m_loaders;

	std::unordered_map<CacheKey, std::weak_ptr<Asset>, CacheKeyHasher> m_cache;
	std::unordered_map<CacheKey, std::shared_future<std::shared_ptr<Asset>>, CacheKeyHasher> m_loading; // 읽는 중인 에셋 (m_cacheMutex로 보호)
	mutable std::mutex m_cacheMutex;
	static constexpr std::size_t kMinSweepThreshold = 64;
	std::size_t m_sweepThreshold{ kMinSweepThreshold }; // 캐시가 이만큼 넘게 커지면 빈 항목을 치운다
};