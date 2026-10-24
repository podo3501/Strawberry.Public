export module Service.AssetMetaRegistry;

import std;
import Core.Type;
import Service.AssetAsyncHelper;
import Contract.Common.Asset;
import Contract.Asset.Service;

export class AssetMetaRegistry : public IAssetMetaRegistry
{
public:
    ~AssetMetaRegistry() override = default;
    explicit AssetMetaRegistry(IAssetAsyncLoader* loader);

	bool Initialize() override;
	bool Scan(const std::filesystem::path& resPath) override;
	std::shared_ptr<Asset> GetMeta(const Core::ResourceID& resID) const override;

private:
    bool RegisterMetaType(std::string_view originalExt, Core::TypeID metaTypeID);

    IAssetAsyncLoader* m_asyncLoader{ nullptr };
    std::unordered_map<std::string, Core::TypeID> m_extToMetaType;
    std::unordered_map<Core::ResourceID, std::shared_ptr<Asset>> m_metaAssets;
};