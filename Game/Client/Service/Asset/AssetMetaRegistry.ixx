export module Service.AssetMetaRegistry;

import std;
import Core.ResourceID;
import Core.TypeHierarchy;
import Core.Utils;
import Service.IAssetAsyncLoader;
import Service.AssetAsyncHelper;
import Contract.Asset.AssetData;
import Contract.Asset;
import Service.Asset.ImageExtensions;

export class AssetMetaRegistry : public IAssetMetaRegistry
{
public:
    virtual ~AssetMetaRegistry() override = default;
    AssetMetaRegistry() = delete;

    static std::unique_ptr<AssetMetaRegistry> Create(IAssetAsyncLoader* loader) noexcept
    {
        return std::unique_ptr<AssetMetaRegistry>(new AssetMetaRegistry(loader));
    }

    bool Initialize()
    {
        const auto typeID = Core::GetTypeID<TextureMetaAsset>();

        return std::ranges::all_of(ImageSupportedExtensions,
            [&](const auto& ext)
            {
                return RegisterMetaType(ext, typeID);
            });
    }

    bool Scan(const std::filesystem::path& resPath)
    {
        std::vector<std::filesystem::path> metaPaths;
        for (auto& entry : std::filesystem::recursive_directory_iterator(resPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".meta")
                metaPaths.push_back(std::filesystem::relative(entry.path(), resPath));
        }

        if (metaPaths.empty())
            return true;

        std::unordered_map<Core::TypeID, std::vector<std::pair<Core::ResourceID, std::filesystem::path>>> grouped;
        for (auto& metaPath : metaPaths)
        {
            std::string originalPathStr = StripMetaSuffix(metaPath.string());
            auto originalExt = Core::GetExtension(originalPathStr);

            auto it = m_extToMetaType.find(Core::ToLower(originalExt));
            if (it == m_extToMetaType.end())
                continue; // 등록 안 된 확장자의 .meta -> skip

            auto originalResID = Core::ResourceID::MakePath(originalPathStr);
            grouped[it->second].push_back({ originalResID, metaPath });
        }

        bool allSucceeded = true;
        for (auto& [metaTypeID, entries] : grouped)
        {
            std::vector<AssetRequest> requests;
            requests.reserve(entries.size());
            for (auto& [originalResID, metaPath] : entries)
                requests.push_back(MakeRequest(metaTypeID, metaPath.string()));

            auto requestIDs = PushRequests(m_asyncLoader, requests);
            auto results = WaitAll(m_asyncLoader, requestIDs);

            for (size_t i = 0; i < entries.size(); ++i)
            {
                if (!results[i])
                {
                    allSucceeded = false;
                    break;
                }
                m_metaAssets[entries[i].first] = results[i];
            }
        }

        return allSucceeded;
    }

    virtual std::shared_ptr<AssetData> GetMeta(const Core::ResourceID& resID) const override
    {
        auto it = m_metaAssets.find(resID);
        if (it == m_metaAssets.end())
            return nullptr;

        return it->second;
    }

private:
    explicit AssetMetaRegistry(IAssetAsyncLoader* loader)
        : m_asyncLoader{ loader }
    {
    }

    bool RegisterMetaType(std::string_view originalExt, Core::TypeID metaTypeID)
    {
        if (originalExt.empty())
            return false;

        std::string normalized = Core::ToLower(originalExt);
        auto it = m_extToMetaType.find(normalized);
        if (it != m_extToMetaType.end() && it->second != metaTypeID)
            return false;

        m_extToMetaType[normalized] = metaTypeID;
        return true;
    }

    static std::string StripMetaSuffix(std::string path)
    {
        constexpr std::string_view MetaSuffix = ".meta";
        if (path.size() >= MetaSuffix.size() && path.ends_with(MetaSuffix))
            path.erase(path.size() - MetaSuffix.size());

        return path;
    }

private:
    IAssetAsyncLoader* m_asyncLoader{ nullptr };
    std::unordered_map<std::string, Core::TypeID> m_extToMetaType;
    std::unordered_map<Core::ResourceID, std::shared_ptr<AssetData>> m_metaAssets;
};