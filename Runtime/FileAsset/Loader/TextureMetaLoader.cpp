#include <nlohmann/json.hpp>

import std;
import Core.Types;
import Core.Utils;
import Contract.Asset.AssetData;
import Contract.Asset;
import Runtime.Serialization;

template<>
constexpr std::array<const char*, Core::EnumSize<ColorSpace>> Core::EnumToStringMap<ColorSpace>()
{
    return Core::MakeEnumStringMap<ColorSpace>("SRGB", "Linear");
}

template<>
struct JsonTraitsBase<ColorSpace>
{
    static nlohmann::json SerializeToJson(const ColorSpace& data)
    {
        return Core::EnumToString(data);
    }

    static ColorSpace DeserializeFromJson(const nlohmann::json& dataJ)
    {
        return CreateAndFill<ColorSpace>([&dataJ](ColorSpace& data) {
            data = *Core::StringToEnum<ColorSpace>(dataJ);
            });
    }
};

template<>
constexpr std::array<const char*, Core::EnumSize<AlphaSourceState>> Core::EnumToStringMap<AlphaSourceState>()
{
    return Core::MakeEnumStringMap<AlphaSourceState>("Opaque", "Straight", "AlreadyPremultiplied");
}

template<>
struct JsonTraitsBase<AlphaSourceState>
{
    static nlohmann::json SerializeToJson(const AlphaSourceState& data)
    {
        return Core::EnumToString(data);
    }

    static AlphaSourceState DeserializeFromJson(const nlohmann::json& dataJ)
    {
        return CreateAndFill<AlphaSourceState>([&dataJ](AlphaSourceState& data) {
            data = *Core::StringToEnum<AlphaSourceState>(dataJ);
            });
    }
};

template<>
constexpr std::array<const char*, Core::EnumSize<BlendTargetSpace>> Core::EnumToStringMap<BlendTargetSpace>()
{
    return Core::MakeEnumStringMap<BlendTargetSpace>("NonPremultiplied", "Premultiplied", "None");
}

template<>
struct JsonTraitsBase<BlendTargetSpace>
{
    static nlohmann::json SerializeToJson(const BlendTargetSpace& data)
    {
        return Core::EnumToString(data);
    }

    static BlendTargetSpace DeserializeFromJson(const nlohmann::json& dataJ)
    {
        return CreateAndFill<BlendTargetSpace>([&dataJ](BlendTargetSpace& data) {
            data = *Core::StringToEnum<BlendTargetSpace>(dataJ);
            });
    }
};

// --- Serialization Internal Struct ---

struct JsonTextureMeta
{
    ColorSpace colorSpace{ ColorSpace::SRGB };
    bool generateMipmaps{ false };
    AlphaSourceState alphaSourceState{ AlphaSourceState::Straight };
    BlendTargetSpace blendTargetSpace{ BlendTargetSpace::NonPremultiplied };

    void Serialize(Serializer& serializer)
    {
        serializer.Process("ColorSpace", colorSpace);
        serializer.Process("GenerateMipmaps", generateMipmaps);
        serializer.Process("AlphaSourceState", alphaSourceState);
        serializer.Process("BlendTargetSpace", blendTargetSpace);
    }
};

// --- Loader Class Implementation ---

class TextureMetaLoader : public IAssetLoader
{
public:
    virtual ~TextureMetaLoader() override = default;

    virtual std::shared_ptr<AssetData> Load(AssetInput& source) override
    {
        if (source.IsStream()) return nullptr;

        auto& mem = static_cast<MemoryInput&>(source);
        return LoadFromMemory(std::move(mem.buffer));
    }

private:
    std::shared_ptr<TextureMetaAsset> LoadFromMemory(Core::ByteBuffer buffer)
    {
        nlohmann::json rData = nlohmann::json::parse(buffer.begin(), buffer.end());

        JsonTextureMeta jsonMeta;
        Serializer reader{ std::as_const(rData) };
        jsonMeta.Serialize(reader);

        auto texMeta = std::make_shared<TextureMetaAsset>();
        texMeta->colorSpace = jsonMeta.colorSpace;
        texMeta->generateMipmaps = jsonMeta.generateMipmaps;
        texMeta->alphaSourceState = jsonMeta.alphaSourceState;
        texMeta->blendTargetSpace = jsonMeta.blendTargetSpace;

        return texMeta;
    }
};

std::unique_ptr<IAssetLoader> CreateTextureMetaLoader()
{
    return std::make_unique<TextureMetaLoader>();
}
