module Service.Asset;

import :AssetRepository;
import std;
import Core.AssetPath;
import Core.Utils;
import Core.Assert;
import Core.Logger;

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
	// 파일이 없거나 못 읽는 것은 데이터 오류라서 죽지 않고 원인을 남긴 뒤 실패(nullptr)로 돌려준다.
	const auto assetPath = resID.GetPath();
	if (!assetPath)
	{
		if (resID.GetType() == Core::ResourceIDType::Path) // Path 종류인데 저장소 밖을 가리키는 경로
			Core::Logger::Error("AssetRepository: 저장소 밖을 가리키는 경로: {}", resID.GetValue());
		return nullptr;
	}

	const Core::AssetPath& path = *assetPath;
	if (loader->PreferStream())
	{
		auto stream = m_assetStorage->CreateReadStream(path);
		if (!stream)
		{
			Core::Logger::Error("AssetRepository: 파일을 열 수 없음: {}", path.String());
			return nullptr;
		}

		StreamInput source(resID, std::move(stream));
		return loader->Load(source);
	}

	std::vector<std::byte> buffer;
	if (!m_assetStorage->Read(path, buffer))
	{
		Core::Logger::Error("AssetRepository: 파일을 읽을 수 없음: {}", path.String());
		return nullptr;
	}

	MemoryInput source(resID, std::move(buffer));
	return loader->Load(source);
}