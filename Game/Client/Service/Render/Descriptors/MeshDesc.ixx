export module Client.Render.Descriptors:Mesh;

import :Resource;
import Client.Asset.Data;

export struct MeshDesc : public ResourceDesc
{
	using ResourceDesc::ResourceDesc;

	virtual Core::TypeID GetAssetTypeID() const override
	{
		return MeshAsset::StaticTypeID();
	}
};