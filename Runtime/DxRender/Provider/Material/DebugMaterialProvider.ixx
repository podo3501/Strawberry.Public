export module DxRender.Provider:DebugMaterial;

import std;
import Contract.Asset.AssetData;
import Contract.Render.Interfaces;

export class DebugMaterialProvider : public IResourceProvider
{
public:
    virtual ~DebugMaterialProvider() override = default;
    DebugMaterialProvider() noexcept = default;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override;
    virtual void ReleaseResource(std::shared_ptr<IResource> resource) override;
};