export module Service.AssetMetaRegistry;

import std;
import Core.Type;
import Service.AssetAsyncHelper;
import Contract.Asset;

export class AssetMetaRegistry : public IAssetMetaRegistry
{
public:
    virtual ~AssetMetaRegistry() override = default;
    AssetMetaRegistry() = delete;
    static std::unique_ptr<AssetMetaRegistry> Create(IAssetAsyncLoader* loader) noexcept;

    bool Initialize();
    bool Scan(const std::filesystem::path& resPath);

    virtual std::shared_ptr<AssetData> GetMeta(const Core::ResourceID& resID) const override;

private:
    explicit AssetMetaRegistry(IAssetAsyncLoader* loader);
    bool RegisterMetaType(std::string_view originalExt, Core::TypeID metaTypeID);

    IAssetAsyncLoader* m_asyncLoader{ nullptr };
    std::unordered_map<std::string, Core::TypeID> m_extToMetaType;
    std::unordered_map<Core::ResourceID, std::shared_ptr<AssetData>> m_metaAssets;
};