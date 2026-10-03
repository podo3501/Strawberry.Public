export module Runtime.Render.Provider:DebugMaterial;

import std;
import Runtime.Render.Resource;
import Core.Assert;
import Core.TypeHierarchy;
import Client.Asset.Data;
import Client.Render.Interfaces;

export class DebugMaterialProvider : public IResourceProvider
{
public:
    virtual ~DebugMaterialProvider() override = default;
    DebugMaterialProvider() noexcept = default;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override
    {
        if (!asset) return nullptr;

        std::shared_ptr<GridDebugMaterialResource> debugRes;

        auto debugAsset = Core::Cast<DebugMaterialAsset>(asset);
        if (!debugAsset) return nullptr;

        switch (debugAsset->type)
        {
        case DebugMaterialType::Grid:
            debugRes = std::make_shared<GridDebugMaterialResource>();
            break;
        }
        Core::Assert(debugRes);

        return debugRes;
    }

    virtual void ReleaseResource(std::shared_ptr<IResource> resource) override
    {
        // GPU 리소스가 아닌 CPU 메모리 객체이므로 스코프 이탈 시 자동 소멸
        return;
    }
};