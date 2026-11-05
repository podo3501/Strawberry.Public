export module Contract.Render.Service.Descriptors:PbrMaterial;

import :Material;
import Contract.Render.Assets;

export struct PbrMaterialDesc : public MaterialDesc
{
	~PbrMaterialDesc() override = default;
	using MaterialDesc::MaterialDesc; // .material 파일 경로

	Core::TypeID GetAssetTypeID() const override
	{
		return PbrMaterialAsset::StaticTypeID();
	}
};