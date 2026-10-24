export module Contract.Render.Service.Descriptors:Mesh;

import :Resource;
import Contract.Render.Assets;

export struct MeshDesc : public ResourceDesc
{
	~MeshDesc() override = default;
	using ResourceDesc::ResourceDesc;

	Core::TypeID GetAssetTypeID() const override
	{
		return MeshAsset::StaticTypeID();
	}
};