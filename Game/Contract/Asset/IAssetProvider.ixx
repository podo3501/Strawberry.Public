export module Client.Asset.Interfaces:IAssetProvider;

import std;
import Client.Asset.AssetData;
import Core.TypeHierarchy;
import Core.ResourceID;

export struct IAssetProvider
{
	virtual ~IAssetProvider() = default;
	virtual std::shared_ptr<AssetData> Load(Core::TypeID type, const Core::ResourceID& resID) = 0;
};