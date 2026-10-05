export module Client.Render.Descriptors:Font;

import :Resource;
import Client.Asset.Data;

export struct FontDesc : public ResourceDesc
{
	using ResourceDesc::ResourceDesc; // .ttf 파일 경로

	virtual Core::TypeID GetAssetTypeID() const override
	{
		return BinaryAsset::StaticTypeID();
	}
};