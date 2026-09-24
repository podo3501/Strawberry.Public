export module Client.Asset.Contract:IAssetMetaRegistry;

import std;
import Core.ResourceID;
import Client.Asset.Data;

namespace Client
{
	export struct IAssetMetaRegistry
	{
		virtual ~IAssetMetaRegistry() = default;
		virtual std::shared_ptr<AssetData> GetMeta(const Core::ResourceID& resID) const = 0;
	};
}