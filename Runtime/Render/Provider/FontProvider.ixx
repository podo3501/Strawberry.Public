export module Runtime.Render.Provider:Font;

import std;
import Runtime.Render.Resource;
import Client.Asset.Data;
import Client.Render.Interfaces;

export class FontProvider : public IResourceProvider
{
public:
    virtual ~FontProvider() override = default;
    FontProvider() noexcept = default;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override
    {
        if (!asset)
            return nullptr;

        auto binAsset = std::static_pointer_cast<BinaryAsset>(asset);
        if (!binAsset || binAsset->buffer.empty())
            return nullptr;

        auto fontRes = std::make_shared<FontResource>();
        if (!fontRes->Initialize(m_ftLibrary, std::move(binAsset)))
            return nullptr;

        return fontRes;
    }

    virtual void ReleaseResource(std::shared_ptr<IResource> resource) override
    {
        // GPU 리소스가 아닌 CPU 메모리 객체이므로 스코프 이탈 시 자동 소멸
        return;
    }

private:
    FreeTypeLibrary m_ftLibrary;
};