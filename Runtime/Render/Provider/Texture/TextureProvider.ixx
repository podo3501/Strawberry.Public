export module Runtime.Render.Provider:Texture;

import std;
import :IUpdatable;
import :BuiltinTexture;
import :PendingUploadQueue;
import :TextureCreateGraphBuilder;
import :Budget;
import :TextureLoadRequest;
import Runtime.Render.Resource;
import Runtime.Render.Task;
import Runtime.Render.Factory;
import Runtime.Render.Shader;
import Runtime.Render.Core;
import Core.Assert;
import Core.Math;
import Core.Utils;
import Client.Asset.Data;

static std::shared_ptr<TextureAsset> CreateColorAsset(const Core::Color& color, ColorSpace colorSpace)
{
    auto asset = std::make_shared<TextureAsset>();
    asset->size = Core::ToSize(1, 1);
    asset->pixels.resize(sizeof(std::uint32_t));

    // 비트 연산을 통해 엔디안에 무관하게 항상 R, G, B, A 순서로 메모리 배치
    asset->pixels[0] = color.R8();
    asset->pixels[1] = color.G8();
    asset->pixels[2] = color.B8();
    asset->pixels[3] = color.A8();

    asset->colorSpace = colorSpace;
    asset->generateMipmaps = false; //같은 색상이기 때문에 불필요
    asset->isPremultipliedAlpha = false; //ui도 기본은 false로.

    return asset;
}

static std::size_t EstimateBytes(const TextureAsset& asset, const TextureDesc& desc)
{
    std::size_t baseBytes = asset.pixels.size();
    if (!desc.generateMipmaps)
        return baseBytes;

    return static_cast<std::size_t>(baseBytes * 4 / 3); //mip 비용은 정확 계산 대신 안정적인 근사 (1.33x)
}

export class TextureProvider : public IUpdatableProvider
{
public:
    virtual ~TextureProvider() override = default;
    TextureProvider() = delete;

    TextureProvider(
        Device& device,
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory) noexcept
        : m_createBuilder{ device, taskScheduler, resFactory, descFactory }
    {
    }

    virtual void Update(float avgGpuMs) override
    {
        auto uploadBudgetBytes = ComputeBudget(avgGpuMs, ProviderBudget::Texture);

        m_pendingLoads.Flush(uploadBudgetBytes, [this](std::vector<TextureLoadRequest>& batch) {
            m_createBuilder.LoadTextures(batch);
            });
    }

    std::shared_ptr<TextureResource> CreateResource()
    {
        return std::make_shared<TextureResource>();
    }

    bool LoadResource(std::shared_ptr<TextureResource> resource, std::shared_ptr<TextureAsset> asset)
    {
        if (!asset) return false;

        TextureDesc desc{ asset->colorSpace, asset->generateMipmaps, asset->isPremultipliedAlpha };
        resource->SetDesc(desc);

        TextureLoadRequest req;
        req.resource = resource;
        req.asset = asset;
        req.estimatedBytes = EstimateBytes(*asset, resource->GetDesc());

        m_pendingLoads.Push(req);
        return true;
    }

    bool Initialize(ShaderLibrary& shaderLibrary)
    {
        if (!CreateBuiltinTextures()) return false;
        if (!m_createBuilder.Initialize(shaderLibrary)) return false;

        return true;
    }

    std::shared_ptr<TextureResource> GetBuiltinTexture(BuiltinTextureType type) const
    {
        auto idx = Core::ToIndex(type);
        Core::Assert(idx < m_builtinTextures.size());

        return m_builtinTextures[idx];
    }

private:
    bool CreateBuiltinTextures()
    {
        std::array<std::shared_ptr<TextureAsset>, Core::EnumSize<BuiltinTextureType>> builtinAssets;

        builtinAssets[Core::ToIndex(BuiltinTextureType::White)] =
            CreateColorAsset(Core::Color::White, ColorSpace::SRGB); // 흰색
        builtinAssets[Core::ToIndex(BuiltinTextureType::FlatNormal)] =
            CreateColorAsset(Core::Color(0.5f, 0.5f, 1.0f), ColorSpace::Linear); // 평평한 노멀 (128, 128, 255)
        builtinAssets[Core::ToIndex(BuiltinTextureType::DefaultARM)] =
            CreateColorAsset(Core::Color(1.0f, 0.3f, 0.5f), ColorSpace::Linear); // ARM 기본

        for (std::size_t nType = 0; nType < Core::EnumSize<BuiltinTextureType>; ++nType)
        {
            auto tex = CreateBuiltinTexture(builtinAssets[nType]);
            if (!tex) return false;

            m_builtinTextures[nType] = tex;
        }

        return true;
    }

    std::shared_ptr<TextureResource> CreateBuiltinTexture(std::shared_ptr<TextureAsset> asset)
    {
        auto texRes = CreateResource();
        if (!texRes)
            return nullptr;

        if (!LoadResource(texRes, asset))
            return nullptr;

        return texRes;
    }

    TextureCreateGraphBuilder m_createBuilder;
    PendingUploadQueue<TextureLoadRequest> m_pendingLoads;
    std::array<std::shared_ptr<TextureResource>, Core::EnumSize<BuiltinTextureType>> m_builtinTextures;
};