module DxRender.Provider;

import std;
import :DebugMaterial;
import DxRender.Resource;
import Core.Logger;
import Core.TypeHierarchy;
import Contract.Render.Assets;

std::shared_ptr<IResource> DebugMaterialProvider::CreateResource(std::shared_ptr<Asset> asset)
{
    std::shared_ptr<GridDebugMaterialResource> debugRes;

    auto debugAsset = Core::Cast<DebugMaterialAsset>(asset);
    if (!debugAsset)
    {
        Core::Logger::Error("DebugMaterialProvider::CreateResource: 디버그 재질 에셋이 아니다 (에셋 {})", asset ? "다른 종류" : "없음");
        return nullptr;
    }

    switch (debugAsset->type)
    {
    case DebugMaterialType::Grid:
        debugRes = std::make_shared<GridDebugMaterialResource>();
        break;
    }
    if (!debugRes)
    {
        Core::Logger::Error("DebugMaterialProvider::CreateResource: 알 수 없는 디버그 재질 종류 {}", static_cast<int>(debugAsset->type));
        return nullptr;
    }

    return debugRes;
}

void DebugMaterialProvider::ReleaseResource(std::shared_ptr<IResource> resource)
{
    // GPU 리소스가 아닌 CPU 메모리 객체이므로 스코프 이탈 시 자동 소멸
    return;
}