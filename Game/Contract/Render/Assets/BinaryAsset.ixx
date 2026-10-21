export module Contract.Render.Assets:BinaryAsset;

import std;
import Contract.Common.Asset;
import Core.TypeHierarchy;
import Core.Types;

export struct BinaryAsset : public Core::TypeNode<BinaryAsset, Asset>
{
	virtual ~BinaryAsset() = default;

	Core::ByteBuffer buffer;
};