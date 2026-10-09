export module Service.Render.Descriptors:PbrMaterial;

import :Material;
import Contract.Asset.Data;

export struct PbrMaterialDesc : public MaterialDesc
{
	using MaterialDesc::MaterialDesc; // .material 파일 경로

	virtual Core::TypeID GetAssetTypeID() const override
	{
		return PbrMaterialAsset::StaticTypeID();
	}
};