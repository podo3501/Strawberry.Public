export module Client.Asset.Data:BinaryAsset;

import std;
import Client.Asset.AssetData;
import Core.TypeHierarchy;
import Core.Types;

export struct BinaryAsset : public Core::TypeNode<BinaryAsset, AssetData>
{
	virtual ~BinaryAsset() = default;

	Core::ByteBuffer buffer;
};