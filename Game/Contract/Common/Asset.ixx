export module Contract.Common.Asset;

import std;
import Core.TypeHierarchy;

export struct Asset : public Core::TypeRoot<Asset>
{
	virtual ~Asset() = default;
};