export module Contract.Render.Service.Descriptors:Environment;

import :Resource;
import Contract.Render.Assets;

export struct EnvironmentDesc : public ResourceDesc
{
	~EnvironmentDesc() override = default;
	using ResourceDesc::ResourceDesc; // .envmap 매니페스트 경로

	Core::TypeID GetAssetTypeID() const override
	{
		return EnvironmentAsset::StaticTypeID();
	}
};