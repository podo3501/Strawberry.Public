export module DxRender.Provider:DebugMaterial;

import std;
import Contract.Common.Asset;
import Contract.Render.Backend;

export class DebugMaterialProvider : public IResourceProvider
{
public:
    ~DebugMaterialProvider() override = default;
    DebugMaterialProvider() noexcept = default;

    std::shared_ptr<IResource> CreateResource(std::shared_ptr<Asset> asset) override;
    void ReleaseResource(std::shared_ptr<IResource> resource) override;
};