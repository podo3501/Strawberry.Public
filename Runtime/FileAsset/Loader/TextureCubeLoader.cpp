#ifdef _DEBUG
#pragma comment(lib, "ktx_Debug.lib")
#else
#pragma comment(lib, "ktx_Release.lib")
#endif

#define KHRONOS_STATIC
#include <ktx.h>

import std;
import Core.BasicTypes;
import Core.Logger;
import Contract.Common.Asset;
import Contract.Render.Assets;
import Contract.Asset.Backend;

static PixelFormat VkFormatToPixelFormat(std::uint32_t vkFormat)
{
    switch (vkFormat)
    {
    case 122: return PixelFormat::R11G11B10F; // VK_FORMAT_B10G11R11_UFLOAT_PACK32
    case 97:  return PixelFormat::RGBA16F;    // VK_FORMAT_R16G16B16A16_SFLOAT
    case 43:  return PixelFormat::RGBA8;      // VK_FORMAT_R8G8B8A8_UNORM
    default:
        Core::Logger::Error("TextureCubeLoader: 지원하지 않는 VkFormat ({})", vkFormat);
        return PixelFormat::Unknown;
    }
}

struct KtxTexture2Deleter
{
    void operator()(ktxTexture2* tex) const
    {
        ktxTexture_Destroy(ktxTexture(tex));
    }
};

using KtxTexture2Ptr = std::unique_ptr<ktxTexture2, KtxTexture2Deleter>;

class TextureCubeLoader : public IAssetLoader
{
public:
    ~TextureCubeLoader() override = default;

    std::shared_ptr<Asset> Load(AssetInput& source) override
    {
        if (source.IsStream()) return nullptr;

        auto& mem = static_cast<MemoryInput&>(source);
        return LoadFromMemory(std::move(mem.buffer));
    }

private:
    std::shared_ptr<TextureCubeAsset> LoadFromMemory(Core::ByteBuffer buffer)
    {
        // KTX2가 아니거나 헤더보다 짧은 데이터를 libktx에 넘기면 호출할 때마다 메모리가 샌다. 시그니처와 크기를 먼저 확인한다
        constexpr std::array<std::uint8_t, 12> kKtx2Identifier{ 0xAB, 'K', 'T', 'X', ' ', '2', '0', 0xBB, '\r', '\n', 0x1A, '\n' };
        constexpr std::size_t kKtx2HeaderSize = 80;
        if (buffer.size() < kKtx2HeaderSize || std::memcmp(buffer.data(), kKtx2Identifier.data(), kKtx2Identifier.size()) != 0)
        {
            Core::Logger::Error("TextureCubeLoader: KTX2 파일이 아님 ({}바이트)", buffer.size());
            return nullptr;
        }

        ktxTexture2* rawTex = nullptr;
        KTX_error_code result = ktxTexture2_CreateFromMemory(
            reinterpret_cast<const ktx_uint8_t*>(buffer.data()),
            buffer.size(),
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &rawTex);

        if (result != KTX_SUCCESS || !rawTex)
        {
            Core::Logger::Error("TextureCubeLoader: KTX2를 읽지 못함 (KTX 오류 {})", static_cast<int>(result));
            return nullptr;
        }

        KtxTexture2Ptr ktxTex(rawTex);

        if (ktxTexture2_NeedsTranscoding(ktxTex.get()))
        {
            result = ktxTexture2_TranscodeBasis(ktxTex.get(), KTX_TTF_BC7_RGBA, 0);
            if (result != KTX_SUCCESS)
            {
                Core::Logger::Error("TextureCubeLoader: 변환하지 못함 (KTX 오류 {})", static_cast<int>(result));
                return nullptr;
            }
        }

        if (!ktxTex->isCubemap)
        {
            Core::Logger::Error("TextureCubeLoader: 큐브맵이 아님 (이 로더는 큐브맵 전용)");
            return nullptr;
        }

        auto asset = std::make_shared<TextureCubeAsset>();
        asset->format = VkFormatToPixelFormat(ktxTex->vkFormat);
        if (asset->format == PixelFormat::Unknown)
            return nullptr;
        asset->colorSpace = ColorSpace::Linear; // cmgen 출력(skybox/IBL 모두)은 항상 Linear
        asset->width = ktxTex->baseWidth;
        asset->height = ktxTex->baseHeight;
        asset->mipCount = ktxTex->numLevels;
        asset->faceCount = ktxTex->numFaces; // 6
        asset->subImages.resize(static_cast<size_t>(asset->mipCount) * asset->faceCount);

        for (uint32_t face = 0; face < asset->faceCount; ++face)
        {
            for (uint32_t mip = 0; mip < asset->mipCount; ++mip)
            {
                ktx_size_t offset = 0;
                KTX_error_code offsetResult = ktxTexture_GetImageOffset(
                    ktxTexture(ktxTex.get()), mip, /*layer*/ 0, face, &offset);
                if (offsetResult != KTX_SUCCESS)
                {
                    Core::Logger::Error("TextureCubeLoader: 이미지 위치를 구하지 못함 (면 {}, 밉 {})", face, mip);
                    return nullptr;
                }

                ktx_size_t imageSize = ktxTexture_GetImageSize(ktxTexture(ktxTex.get()), mip);
                const uint8_t* srcData = ktxTexture_GetData(ktxTexture(ktxTex.get())) + offset;

                TextureCubeMipFace& sub = asset->subImages[mip + face * asset->mipCount];
                sub.width = std::max(1u, asset->width >> mip);
                sub.height = std::max(1u, asset->height >> mip);
                sub.rowPitch = static_cast<uint32_t>(imageSize / sub.height);
                sub.pixels.assign(srcData, srcData + imageSize);
            }
        }

        return asset;
    }
};

std::unique_ptr<IAssetLoader> CreateTextureCubeLoader()
{
    return std::make_unique<TextureCubeLoader>();
}