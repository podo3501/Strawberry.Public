module DxRender.Provider;

import std;
import :DebugMaterial;
import DxRender.Resource;
import Core.Assert;
import Core.TypeHierarchy;
import Contract.Asset.Data;

std::shared_ptr<IResource> DebugMaterialProvider::CreateResource(std::shared_ptr<AssetData> asset)
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

void DebugMaterialProvider::ReleaseResource(std::shared_ptr<IResource> resource)
{
    // GPU 리소스가 아닌 CPU 메모리 객체이므로 스코프 이탈 시 자동 소멸
    return;
}