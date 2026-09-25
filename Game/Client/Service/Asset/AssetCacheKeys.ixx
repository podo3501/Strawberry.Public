export module Client.Asset.Service:AssetCacheKeys;

import std;
import Core.TypeHierarchy;
import Core.ResourceID;
import Core.Utils;

namespace Client
{
	export inline std::size_t HashCombine(std::size_t h1, std::size_t h2) noexcept
	{
		return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
	}

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
}