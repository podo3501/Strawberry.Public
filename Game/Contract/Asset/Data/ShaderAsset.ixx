export module Client.Asset.Data:ShaderAsset;

import std;
import :AssetData;
import Core.TypeHierarchy;
import Core.Types;

namespace Client
{
	export struct ShaderAsset : public Core::TypeNode<ShaderAsset, AssetData>
	{
		virtual ~ShaderAsset() = default;

		std::string hlslSource; // 파일 전체 원본 소스

		Core::ByteBuffer vs;
		Core::ByteBuffer ps;
		Core::ByteBuffer cs;
	};
}