import std;
import Core.Type;
import Core.Logger;
import Contract.Common.Asset;
import Contract.Render.Assets;
import Contract.Asset.Service;
import Contract.Asset.Backend;

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
    {}

    std::shared_ptr<Asset> Load(AssetInput& source) override
    {
        if (source.IsStream()) return nullptr;

        auto& mem = static_cast<MemoryInput&>(source);
        return LoadFromMemory(mem.resID, std::move(mem.buffer));
    }

private:
    // 환경맵 파일 옆의 파일을 provider로 읽는다. 이름이 올바르지 않거나 읽지 못하거나 타입이 다르면 Error를 남기고 nullptr
    template<typename T>
    std::shared_ptr<T> LoadSibling(const Core::ResourceID& resID, const std::string& key, const std::string& name)
    {
        const auto id = resID.TryMakeSibling(name);
        if (!id)
        {
            Core::Logger::Error("EnvironmentLoader: {}의 파일 이름이 올바르지 않음 ({})", key, name);
            return nullptr;
        }

        auto asset = Core::Cast<T>(m_assetProvider->Load(Core::GetTypeID<T>(), *id));
        if (!asset)
            Core::Logger::Error("EnvironmentLoader: {}의 파일을 읽지 못함 ({})", key, id->GetValue());
        return asset;
    }

    std::shared_ptr<EnvironmentAsset> LoadFromMemory(const Core::ResourceID& resID, Core::ByteBuffer buffer)
    {
        std::string text(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        auto kv = ParseSimpleKeyValue(text);

        // 세 항목 모두 필수다 (렌더가 환경 리소스를 만들 때 셋 다 필요하다)
        for (const char* key : { "Skybox", "Reflection", "Irradiance" })
        {
            if (!kv.contains(key))
            {
                Core::Logger::Error("EnvironmentLoader: 필수 항목이 없음 ({})", key);
                return nullptr;
            }
        }

        auto asset = std::make_shared<EnvironmentAsset>();
        asset->skybox = LoadSibling<TextureCubeAsset>(resID, "Skybox", kv["Skybox"]);
        asset->reflection = LoadSibling<TextureCubeAsset>(resID, "Reflection", kv["Reflection"]);
        asset->irradiance = LoadSibling<SphericalHarmonicsAsset>(resID, "Irradiance", kv["Irradiance"]);

        if (!asset->skybox || !asset->reflection || !asset->irradiance)
            return nullptr;

        return asset;
    }

private:
    IAssetProvider* m_assetProvider{ nullptr };
};

std::unique_ptr<IAssetLoader> CreateEnvironmentLoader(IAssetProvider* assetProvider)
{
    return std::make_unique<EnvironmentLoader>(assetProvider);
}