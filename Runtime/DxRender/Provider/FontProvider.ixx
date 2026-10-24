export module DxRender.Provider:Font;

import std;
import DxRender.Resource;
import Contract.Common.Asset;
import Contract.Render.Backend;

export class FontProvider : public IResourceProvider
{
public:
    ~FontProvider() override = default;
    FontProvider() noexcept = default;

    std::shared_ptr<IResource> CreateResource(std::shared_ptr<Asset> asset) override;
    void ReleaseResource(std::shared_ptr<IResource> resource) override;

private:
    FreeTypeLibrary m_ftLibrary;
};