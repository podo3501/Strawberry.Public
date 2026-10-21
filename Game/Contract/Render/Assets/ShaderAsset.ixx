export module Contract.Render.Assets:ShaderAsset;

import std;
import Contract.Common.Asset;
import Core.TypeHierarchy;
import Core.Types;

export struct ShaderAsset : public Core::TypeNode<ShaderAsset, Asset>
{
	virtual ~ShaderAsset() = default;

	std::string hlslSource; // 파일 전체 원본 소스

	Core::ByteBuffer vs;
	Core::ByteBuffer ps;
	Core::ByteBuffer cs;
};