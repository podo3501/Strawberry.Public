module Service.Asset;

import :AssetRepository;
import std;
import Core.Utils;
import Core.Assert;

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

std::shared_ptr<Asset> AssetRepository::Load(Core::TypeID type, const Core::ResourceID& resID)
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

std::shared_ptr<Asset> AssetRepository::LoadWithSource(IAssetLoader* loader, const Core::ResourceID& resID)
{
	if (resID.GetType() != Core::ResourceIDType::Path)
		return nullptr;

	const auto path = std::filesystem::path(resID.GetValue());
	if (loader->PreferStream())
	{
		auto stream = m_assetStorage->CreateReadStream(path);
		Core::Assert(stream != nullptr); // 파일이 없거나 열 수 없음
		if (!stream) return nullptr;

		StreamInput source(resID, std::move(stream));
		return loader->Load(source);
	}

	std::vector<std::byte> buffer;
	const bool readOk = m_assetStorage->Read(path, buffer);
	Core::Assert(readOk); // 파일이 없거나 읽기 실패
	if (!readOk) return nullptr;

	MemoryInput source(resID, std::move(buffer));
	return loader->Load(source);
}