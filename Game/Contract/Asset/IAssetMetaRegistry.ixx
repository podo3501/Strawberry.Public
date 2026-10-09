export module Contract.Asset.Interfaces:IAssetMetaRegistry;

import std;
import Core.ResourceID;
import Contract.Asset.AssetData;

export struct IAssetMetaRegistry
{
	virtual ~IAssetMetaRegistry() = default;
	virtual std::shared_ptr<AssetData> GetMeta(const Core::ResourceID& resID) const = 0;
};