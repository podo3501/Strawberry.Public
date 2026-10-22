export module DxRender.Provider:DebugMaterial;

import std;
import Contract.Common.Asset;
import Contract.Render.Backend;

export class DebugMaterialProvider : public IResourceProvider
{
public:
    virtual ~DebugMaterialProvider() override = default;
    DebugMaterialProvider() noexcept = default;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<Asset> asset) override;
    virtual void ReleaseResource(std::shared_ptr<IResource> resource) override;
};