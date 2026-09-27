export module Client.Asset.Interfaces:IAssetMetaRegistry;

import std;
import Core.ResourceID;
import Client.Asset.AssetData;

export struct IAssetMetaRegistry
{
	virtual ~IAssetMetaRegistry() = default;
	virtual std::shared_ptr<AssetData> GetMeta(const Core::ResourceID& resID) const = 0;
};