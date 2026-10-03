export module Runtime.Render.Provider:TextureCube;

import std;
import :IUpdatable;
import :TextureCubeLoadRequest;
import :PendingUploadQueue;
import :TextureCubeCreateGraphBuilder;
import :Budget;
import Runtime.Render.Resource;
import Runtime.Render.Task;
import Runtime.Render.Factory;
import Client.Asset.Data;

static std::size_t EstimateBytes(const TextureCubeAsset& asset)
{
    std::size_t total = 0;
    for (const auto& sub : asset.subImages)
        total += sub.pixels.size();
    return total;
}

export class TextureCubeProvider : public IUpdatableProvider
{
public:
    virtual ~TextureCubeProvider() override = default;

    TextureCubeProvider(
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory) noexcept :
        m_createBuilder{ taskScheduler, resFactory, descFactory }
    {}

    virtual void Update(float avgGpuMs) override
    {
        auto uploadBudgetBytes = ComputeBudget(avgGpuMs, ProviderBudget::TextureCube);

        m_pendingLoads.Flush(uploadBudgetBytes, [this](std::vector<TextureCubeLoadRequest>& batch) {
            m_createBuilder.LoadTextureCubes(batch);
            });
    }

    std::shared_ptr<TextureCubeResource> CreateResource()
    {
        return std::make_shared<TextureCubeResource>();
    }

    bool LoadResource(
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

private:
    TextureCubeCreateGraphBuilder m_createBuilder;
    PendingUploadQueue<TextureCubeLoadRequest> m_pendingLoads;
};