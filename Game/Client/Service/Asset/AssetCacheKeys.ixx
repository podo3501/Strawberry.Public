export module Service.Asset:AssetCacheKeys;

import std;
import Core.Type;
import Core.Utils;

export struct LoaderKey
{
	Core::TypeID type;
	std::string ext;

	bool operator==(const LoaderKey& other) const = default;
};

export struct LoaderKeyHasher
{
	std::size_t operator()(const LoaderKey& k) const
	{
		return Core::HashOf(k.type, k.ext);
	}
};

export struct CacheKey
{
	Core::ResourceID resID;
	Core::TypeID type;

	bool operator==(const CacheKey& other) const = default;
};

export struct CacheKeyHasher
{
	std::size_t operator()(const CacheKey& k) const
	{
		return Core::HashOf(k.resID, k.type);
	}
};