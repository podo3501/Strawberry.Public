export module Contract.AssetAsyncTypes;

import Core.Type;
import Contract.Common.Asset;

export using AssetPtr = std::shared_ptr<Asset>;

export using AssetRequestID = std::uint64_t;
export inline constexpr AssetRequestID InvalidAssetRequestID = 0;

export struct AssetRequest
{
	Core::ResourceID resID;
	Core::TypeID type{ Core::InvalidTypeID };
};