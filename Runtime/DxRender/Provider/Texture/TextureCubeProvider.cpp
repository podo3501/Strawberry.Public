module DxRender.Provider;

import :TextureCube;
import :Budget;

static std::size_t EstimateBytes(const TextureCubeAsset& asset)
{
    std::size_t total = 0;
    for (const auto& sub : asset.subImages)
        total += sub.pixels.size();
    return total;
}

TextureCubeProvider::TextureCubeProvider(
    TaskScheduler& taskScheduler,
    ResourceFactory& resFactory,
    DescriptorFactory& descFactory) noexcept :
    m_createBuilder{ taskScheduler, resFactory, descFactory }
{}

void TextureCubeProvider::Update(float avgGpuMs)
{
    auto uploadBudgetBytes = ComputeBudget(avgGpuMs, ProviderBudget::TextureCube);

    m_pendingLoads.Flush(uploadBudgetBytes, [this](std::vector<TextureCubeLoadRequest>& batch) {
        m_createBuilder.LoadTextureCubes(batch);
        });
}

std::shared_ptr<TextureCubeResource> TextureCubeProvider::CreateResource()
{
    return std::make_shared<TextureCubeResource>();
}

bool TextureCubeProvider::LoadResource(
    std::shared_ptr<TextureCubeResource> resource,
    std::shared_ptr<TextureCubeAsset> asset)
{
    if (!asset) return false;

    TextureCubeDesc desc{ asset->colorSpace }; // 항상 Linear
    resource->SetDesc(desc);

    TextureCubeLoadRequest req;
    req.resource = resource;
    req.asset = asset;
    req.estimatedBytes = EstimateBytes(*asset);

    m_pendingLoads.Push(req);
    return true;
}