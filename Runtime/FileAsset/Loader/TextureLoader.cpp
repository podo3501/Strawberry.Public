#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

import std;
import Core.Type;
import Core.Math;
import Core.Logger;
import Contract.Common.Asset;
import Contract.Render.Assets;
import Contract.Asset.Service;
import Contract.Asset.Backend;

static void ApplyPremultipliedAlpha(std::vector<std::uint8_t>& pixels)
{
    for (size_t i = 0; i + 3 < pixels.size(); i += 4)
    {
        const float a = pixels[i + 3] / 255.0f;
        pixels[i + 0] = static_cast<std::uint8_t>(pixels[i + 0] * a + 0.5f);
        pixels[i + 1] = static_cast<std::uint8_t>(pixels[i + 1] * a + 0.5f);
        pixels[i + 2] = static_cast<std::uint8_t>(pixels[i + 2] * a + 0.5f);
    }
}

class TextureLoader : public IAssetLoader
{
public:
    ~TextureLoader() override = default;

    explicit TextureLoader(IAssetMetaRegistry* metaRegistry) noexcept
        : m_metaRegistry{ metaRegistry }
    {
    }

    std::shared_ptr<Asset> Load(AssetInput& source) override
    {
        if (source.IsStream()) return nullptr;

        auto& mem = static_cast<MemoryInput&>(source);
        return LoadFromMemory(mem.resID, std::move(mem.buffer));
    }

private:
    std::shared_ptr<TextureAsset> LoadFromMemory(const Core::ResourceID& resID, Core::ByteBuffer buffer)
    {
        if (buffer.empty() || buffer.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
            Core::Logger::Error("TextureLoader: 이미지 크기가 올바르지 않음 ({}바이트)", buffer.size());
            return nullptr;
        }

        int width, height, channels;

        unsigned char* data = stbi_load_from_memory(
            reinterpret_cast<const unsigned char*>(buffer.data()),
            static_cast<int>(buffer.size()),
            &width,
            &height,
            &channels,
            4 // RGBA 강제 (jpg 등의 채널 유무 일과 대응)
        );

        if (!data)
        {
            Core::Logger::Error("TextureLoader: 이미지를 읽지 못함 ({})", stbi_failure_reason());
            return nullptr;
        }

        auto asset = std::make_shared<TextureAsset>();
        asset->size = Core::ToSize(width, height);
        asset->stride = static_cast<uint32_t>(width * 4);
        asset->format = PixelFormat::RGBA8;

        const std::size_t size = static_cast<std::size_t>(width) * height * 4;
        asset->pixels.assign(data, data + size);

        stbi_image_free(data);

        AlphaSourceState source = AlphaSourceState::Straight;
        BlendTargetSpace target = BlendTargetSpace::NonPremultiplied;

        if (m_metaRegistry)
        {
            auto metaData = m_metaRegistry->GetMeta(resID);
            if (auto textureMeta = Core::Cast<TextureMetaAsset>(metaData))
            {
                asset->colorSpace = textureMeta->colorSpace;
                asset->generateMipmaps = textureMeta->generateMipmaps;

                source = textureMeta->alphaSourceState;
                target = textureMeta->blendTargetSpace;
            }
        }

        bool resultIsPMA = false;
        if (source == AlphaSourceState::Opaque || target == BlendTargetSpace::None)
        {
            resultIsPMA = false;
        }
        else if (source == AlphaSourceState::Straight && target == BlendTargetSpace::Premultiplied)
        {
            ApplyPremultipliedAlpha(asset->pixels);
            resultIsPMA = true;
        }
        else if (source == AlphaSourceState::AlreadyPremultiplied && target == BlendTargetSpace::NonPremultiplied)
        {
            Core::Logger::Error("TextureLoader: 이미 곱해 둔 알파를 곱하지 않은 알파로 되돌리는 조합은 지원하지 않음 ({})", resID.GetValue());
            return nullptr;
        }
        else
        {
            resultIsPMA = (target == BlendTargetSpace::Premultiplied);
        }

        asset->isPremultipliedAlpha = resultIsPMA;
        return asset;
    }

private:
    IAssetMetaRegistry* m_metaRegistry{ nullptr };
};

std::unique_ptr<IAssetLoader> CreateImageTextureLoader(IAssetMetaRegistry* metaRegistry)
{
    return std::make_unique<TextureLoader>(metaRegistry);
}