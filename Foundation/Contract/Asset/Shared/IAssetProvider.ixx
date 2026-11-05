export module Contract.Asset.Shared:IAssetProvider;

import std;
import Contract.Common.Asset;
import Core.Type;

export struct IAssetProvider
{
	virtual ~IAssetProvider() = default;
	virtual std::shared_ptr<Asset> Load(Core::TypeID type, const Core::ResourceID& resID) = 0;
};