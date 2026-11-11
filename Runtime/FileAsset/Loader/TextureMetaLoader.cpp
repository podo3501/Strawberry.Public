#include <nlohmann/json.hpp>

import std;
import Serialization;
import Core.Logger;
import Core.BasicTypes;
import Core.Utils;
import Contract.Common.Asset;
import Contract.Render.Assets;
import Contract.Asset.Backend;

template<>
constexpr std::array<const char*, Core::EnumSize<ColorSpace>> Core::EnumToStringMap<ColorSpace>()
{
    return Core::MakeEnumStringMap<ColorSpace>("SRGB", "Linear");
}

template<>
constexpr std::array<const char*, Core::EnumSize<AlphaSourceState>> Core::EnumToStringMap<AlphaSourceState>()
{
    return Core::MakeEnumStringMap<AlphaSourceState>("Opaque", "Straight", "AlreadyPremultiplied");
}

template<>
constexpr std::array<const char*, Core::EnumSize<BlendTargetSpace>> Core::EnumToStringMap<BlendTargetSpace>()
{
    return Core::MakeEnumStringMap<BlendTargetSpace>("NonPremultiplied", "Premultiplied", "None");
}

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
    ~TextureMetaLoader() override = default;

    std::shared_ptr<Asset> Load(AssetInput& source) override
    {
        if (source.IsStream()) return nullptr;

        auto& mem = static_cast<MemoryInput&>(source);
        return LoadFromMemory(mem.buffer);
    }

private:
    std::shared_ptr<TextureMetaAsset> LoadFromMemory(const Core::ByteBuffer& buffer)
    {
        nlohmann::json rData = nlohmann::json::parse(buffer.begin(), buffer.end(), nullptr, false);   // 예외 없이 실패를 알린다
        if (rData.is_discarded())
        {
            Core::Logger::Error("TextureMetaLoader: JSON 문법이 올바르지 않음");
            return nullptr;
        }

        JsonTextureMeta jsonMeta;
        auto reader = Serializer::ForRead(rData);
        jsonMeta.Serialize(reader);
        if (reader.HasError())
        {
            Core::Logger::Error("TextureMetaLoader: 값을 읽지 못함");
            return nullptr;
        }

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
