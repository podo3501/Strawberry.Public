export module Client.Asset.Service:AssetRepository;

import std;
import :AssetCacheKeys;
import :AssetLoaderDesc;
import Client.Asset.Contract;
import Core.ResourceID;
import Core.Utils;
import Core.Assert;

export class AssetRepository : public IAssetProvider
{
public:
	~AssetRepository() override = default;
	explicit AssetRepository(IAssetStorage* assetStorage) noexcept;

	bool RegisterLoader(AssetLoaderDesc desc);
	std::shared_ptr<AssetData> Load(Core::TypeID type, const Core::ResourceID& resID) override;

private:
	std::shared_ptr<AssetData> LoadWithSource(IAssetLoader* loader, const Core::ResourceID& resID);

	IAssetStorage* m_assetStorage{ nullptr };
	std::unordered_map<LoaderKey, std::unique_ptr<IAssetLoader>, LoaderKeyHasher> m_loaders;

	std::unordered_map<CacheKey, std::weak_ptr<AssetData>, CacheKeyHasher> m_cache;
	mutable std::mutex m_cacheMutex;
};

AssetRepository::AssetRepository(IAssetStorage* assetStorage) noexcept :
	m_assetStorage{ assetStorage }
{}

bool AssetRepository::RegisterLoader(AssetLoaderDesc desc)
{
	if (!desc.loader) return false;

	std::string normalized = Core::ToLowerCopy(desc.extension);
	LoaderKey loaderKey{ desc.type, normalized };

	m_loaders[loaderKey] = std::move(desc.loader);
	return true;
}

std::shared_ptr<AssetData> AssetRepository::Load(Core::TypeID type, const Core::ResourceID& resID)
{
	CacheKey cacheKey{ resID, type };

	{
		std::lock_guard lock(m_cacheMutex);
		auto it = m_cache.find(cacheKey);
		if (it != m_cache.end())
		{
			if (auto cached = it->second.lock())
				return cached;
		}
	}

	auto ext = std::string(Core::GetExtension(resID.GetValue()));
	std::string normalized = Core::ToLowerCopy(ext);
	LoaderKey loaderKey{ type, normalized };

	IAssetLoader* loader = nullptr;
	auto loaderIt = m_loaders.find(loaderKey);
	Core::Assert(loaderIt != m_loaders.end()); // 파일 확장자에 맞는 로더가 없거나 타입 불일치 시 Assert
	if (loaderIt == m_loaders.end())
		return nullptr;

	loader = loaderIt->second.get();
	auto asset = LoadWithSource(loader, resID);
	if (!asset) return nullptr;

	{
		std::lock_guard lock(m_cacheMutex);

		auto it = m_cache.find(cacheKey);
		if (it != m_cache.end()) // 다른 스레드가 먼저 캐싱했을 가능성 처리
		{
			if (auto existing = it->second.lock())
				return existing;
		}
		m_cache[cacheKey] = asset;
	}

	return asset;
}

std::shared_ptr<AssetData> AssetRepository::LoadWithSource(IAssetLoader* loader, const Core::ResourceID& resID)
{
	if (resID.GetType() != Core::ResourceIDType::Path)
		return nullptr;

	const auto path = std::filesystem::path(resID.GetValue());
	if (loader->PreferStream())
	{
		auto stream = m_assetStorage->CreateReadStream(path);
		if (!stream) return nullptr;

		StreamInput source(resID, std::move(stream));
		return loader->Load(source);
	}

	std::vector<std::byte> buffer;
	if (!m_assetStorage->Read(path, buffer)) return nullptr;

	MemoryInput source(resID, std::move(buffer));
	return loader->Load(source);
}