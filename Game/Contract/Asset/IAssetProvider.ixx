export module Contract.Asset.Interfaces:IAssetProvider;

import std;
import Contract.Asset.AssetData;
import Core.TypeHierarchy;
import Core.ResourceID;

export struct IAssetProvider
{
	virtual ~IAssetProvider() = default;
	virtual std::shared_ptr<AssetData> Load(Core::TypeID type, const Core::ResourceID& resID) = 0;
};