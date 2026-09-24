export module Client.Asset.Contract:IAssetProvider;

import std;
import Client.Asset.Data;
import Core.TypeHierarchy;
import Core.ResourceID;

namespace Client
{
	export struct IAssetProvider
	{
		virtual ~IAssetProvider() = default;
		virtual std::shared_ptr<AssetData> Load(Core::TypeID type, const Core::ResourceID& resID) = 0;
	};
}