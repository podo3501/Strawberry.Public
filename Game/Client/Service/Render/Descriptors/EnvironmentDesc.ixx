export module Service.Render.Descriptors:Environment;

import :Resource;
import Contract.Render.Assets;

export struct EnvironmentDesc : public ResourceDesc
{
	using ResourceDesc::ResourceDesc; // .envmap 매니페스트 경로

	virtual Core::TypeID GetAssetTypeID() const override
	{
		return EnvironmentAsset::StaticTypeID();
	}
};