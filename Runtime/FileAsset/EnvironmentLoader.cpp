import std;
import Core.Assert;
import Core.Types;
import Core.TypeHierarchy;
import Core.ResourceID;
import Contract.Asset;

static std::unordered_map<std::string, std::string> ParseSimpleKeyValue(const std::string& text)
{
    std::unordered_map<std::string, std::string> result;
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line))
    {
        // 빈 줄이나 주석(#) 무시
        if (line.empty() || line[0] == '#')
            continue;

        auto eqPos = line.find('=');
        if (eqPos == std::string::npos)
            continue;

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        // 앞뒤 공백/개행 제거 (Windows CRLF 대응 포함)
        auto trim = [](std::string& s)
            {
                while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
                    s.pop_back();
                size_t start = 0;
                while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
                    ++start;
                s = s.substr(start);
            };
        trim(key);
        trim(value);

        result[key] = value;
    }
    return result;
}

class EnvironmentLoader : public IAssetLoader
{
public:
    ~EnvironmentLoader() override = default;

    explicit EnvironmentLoader(IAssetProvider* assetProvider) noexcept
        : m_assetProvider{ assetProvider }
    {
    }

    std::shared_ptr<AssetData> Load(AssetInput& source) override
    {
        if (source.IsStream()) return nullptr;

        auto& mem = static_cast<MemoryInput&>(source);
        return LoadFromMemory(mem.resID, std::move(mem.buffer));
    }

private:
    std::shared_ptr<EnvironmentAsset> LoadFromMemory(const Core::ResourceID& resID, Core::ByteBuffer buffer)
    {
        std::string text(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        auto kv = ParseSimpleKeyValue(text);

        auto skyboxIt = kv.find("Skybox");
        auto reflectionIt = kv.find("Reflection");
        auto irradianceIt = kv.find("Irradiance");

        if (skyboxIt == kv.end() || reflectionIt == kv.end())
        {
            Core::Assert(false); // 필수 필드 누락
            return nullptr;
        }

        auto asset = std::make_shared<EnvironmentAsset>();

        auto skybox = m_assetProvider->Load(Core::GetTypeID<TextureCubeAsset>(),
            resID.MakeSibling(skyboxIt->second));
        asset->skybox = Core::Cast<TextureCubeAsset>(skybox);

        auto reflection = m_assetProvider->Load(Core::GetTypeID<TextureCubeAsset>(),
            resID.MakeSibling(reflectionIt->second));
        asset->reflection = Core::Cast<TextureCubeAsset>(reflection);

        if (!asset->skybox || !asset->reflection)
            return nullptr;

        if (irradianceIt != kv.end())
        {
            auto irradiance = m_assetProvider->Load(Core::GetTypeID<SphericalHarmonicsAsset>(),
                resID.MakeSibling(irradianceIt->second));
            asset->irradiance = Core::Cast<SphericalHarmonicsAsset>(irradiance);
        }

        return asset;
    }

private:
    IAssetProvider* m_assetProvider{ nullptr };
};

std::unique_ptr<IAssetLoader> CreateEnvironmentLoader(IAssetProvider* assetProvider)
{
    return std::make_unique<EnvironmentLoader>(assetProvider);
}