export module Service.AssetAsyncTypes;

import std;
import Core.ResourceID;
import Core.TypeHierarchy;
import Contract.Asset.AssetData;

export using AssetPtr = std::shared_ptr<AssetData>;

export using AssetRequestID = std::uint64_t;
export inline constexpr AssetRequestID InvalidAssetRequestID = 0;

export struct AssetRequest
{
	Core::ResourceID resID;
	Core::TypeID type{ Core::InvalidTypeID };
};
