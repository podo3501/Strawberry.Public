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

	if (m_loaders.contains(loaderKey))
	{
		Core::Logger::Error("AssetRepository: 같은 종류와 확장자의 로더가 이미 등록됨: {}", normalized);
		return false;
	}

	m_loaders.emplace(std::move(loaderKey), std::move(desc.loader));
	return true;
}

std::shared_ptr<Asset> AssetRepository::Load(Core::TypeID type, const Core::ResourceID& resID)
{
	// 파일을 가리키는 ID만 읽을 수 있다. 그 밖의 ID는 데이터 오류라서 죽지 않고 원인을 남긴 뒤 실패(nullptr)로 돌려준다.
	const auto assetPath = resID.GetPath();
	if (!assetPath)
	{
		Core::Logger::Error("AssetRepository: 파일을 가리키지 않거나 저장소 밖을 가리키는 ID: {}", resID.String());
		return nullptr;
	}

	CacheKey cacheKey{ resID, type };

	std::promise<std::shared_ptr<Asset>> promise;
	{
		std::unique_lock lock(m_cacheMutex);
		auto it = m_cache.find(cacheKey);
		if (it != m_cache.end())
		{
			if (auto cached = it->second.lock())
				return cached;
		}

		auto loadingIt = m_loading.find(cacheKey);
		if (loadingIt != m_loading.end()) // 다른 스레드가 같은 에셋을 읽는 중이면 그 결과를 기다린다
		{
			auto loading = loadingIt->second;
			lock.unlock();
			return loading.get();
		}

		m_loading.emplace(cacheKey, promise.get_future().share());
	}

	auto asset = LoadUncached(type, resID, *assetPath);

	{
		std::lock_guard lock(m_cacheMutex);
		if (asset)
		{
			m_cache[cacheKey] = asset;
			SweepExpiredCache();
		}
		m_loading.erase(cacheKey);
	}

	promise.set_value(asset); // 실패(nullptr)도 기다리던 쪽에 그대로 전한다. 실패는 캐시에 남지 않아 다음 요청은 다시 읽는다
	return asset;
}

std::shared_ptr<Asset> AssetRepository::LoadUncached(Core::TypeID type, const Core::ResourceID& resID, const Core::AssetPath& path)
{
	auto ext = std::string(Core::GetExtension(resID.GetValue()));
	std::string normalized = Core::ToLowerCopy(ext);
	LoaderKey loaderKey{ type, normalized };

	auto loaderIt = m_loaders.find(loaderKey);
	Core::Assert(loaderIt != m_loaders.end()); // 파일 확장자에 맞는 로더가 없거나 타입 불일치 시 Assert
	if (loaderIt == m_loaders.end())
		return nullptr;

	return LoadWithSource(loaderIt->second.get(), resID, path);
}

void AssetRepository::SweepExpiredCache()
{
	if (m_cache.size() <= m_sweepThreshold)
		return;

	std::erase_if(m_cache, [](const auto& entry) { return entry.second.expired(); });
	m_sweepThreshold = std::max(kMinSweepThreshold, m_cache.size() * 2);
}

std::shared_ptr<Asset> AssetRepository::LoadWithSource(IAssetLoader* loader, const Core::ResourceID& resID, const Core::AssetPath& path)
{
	// 파일이 없거나 못 읽는 것은 데이터 오류라서 죽지 않고 원인을 남긴 뒤 실패(nullptr)로 돌려준다.
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