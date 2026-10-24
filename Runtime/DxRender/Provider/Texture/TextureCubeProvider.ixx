export module DxRender.Provider:TextureCube;

import std;
import :IUpdatable;
import :TextureCubeLoadRequest;
import :PendingUploadQueue;
import :TextureCubeCreateGraphBuilder;
import DxRender.Resource;
import DxRender.Task;
import DxRender.Factory;
import Contract.Render.Assets;

export class TextureCubeProvider : public IUpdatableProvider
{
public:
    ~TextureCubeProvider() override = default;
    TextureCubeProvider(
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory) noexcept;
    void Update(float avgGpuMs) override;

    std::shared_ptr<TextureCubeResource> CreateResource();
    bool LoadResource(
        std::shared_ptr<TextureCubeResource> resource,
        std::shared_ptr<TextureCubeAsset> asset);

private:
    TextureCubeCreateGraphBuilder m_createBuilder;
    PendingUploadQueue<TextureCubeLoadRequest> m_pendingLoads;
};