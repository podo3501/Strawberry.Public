export module Client.Asset.Data:BinaryAsset;

import std;
import :AssetData;
import Core.TypeHierarchy;
import Core.Types;

namespace Client
{
	export struct BinaryAsset : public Core::TypeNode<BinaryAsset, AssetData>
	{
		virtual ~BinaryAsset() = default;

		Core::ByteBuffer buffer;
	};
}