export module Contract.Render.Service.Descriptors:Font;

import :Resource;
import Contract.Render.Assets;

export struct FontDesc : public ResourceDesc
{
	~FontDesc() override = default;
	using ResourceDesc::ResourceDesc; // .ttf 파일 경로

	Core::TypeID GetAssetTypeID() const override
	{
		return BinaryAsset::StaticTypeID();
	}
};