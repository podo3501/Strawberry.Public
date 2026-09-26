export module Client.Asset.Data:AssetData;

import std;
import Core.TypeHierarchy;

export struct AssetData : public Core::TypeRoot<AssetData>
{
	virtual ~AssetData() = default;
};