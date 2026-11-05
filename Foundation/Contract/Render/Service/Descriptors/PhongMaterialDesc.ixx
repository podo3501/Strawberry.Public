export module Contract.Render.Service.Descriptors:PhongMaterial;

import :Material;
import Contract.Render.Assets;

export struct PhongMaterialDesc : public MaterialDesc
{
	~PhongMaterialDesc() override = default;
	using MaterialDesc::MaterialDesc; // .material 파일 경로

	Core::TypeID GetAssetTypeID() const override
	{
		return PhongMaterialAsset::StaticTypeID();
	}
};