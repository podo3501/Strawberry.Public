export module DxRender.Provider:Font;

import std;
import DxRender.Resource;
import Contract.Asset.AssetData;
import Contract.Render.Interfaces;

export class FontProvider : public IResourceProvider
{
public:
    virtual ~FontProvider() override = default;
    FontProvider() noexcept = default;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override;
    virtual void ReleaseResource(std::shared_ptr<IResource> resource) override;

private:
    FreeTypeLibrary m_ftLibrary;
};