module DxRender.Provider;

import std;
import :Font;
import DxRender.Resource;
import Contract.Render.Assets;
import Core.Logger;
import Core.TypeHierarchy;

std::shared_ptr<IResource> FontProvider::CreateResource(std::shared_ptr<Asset> asset)
{
    auto binAsset = Core::Cast<BinaryAsset>(asset);
    if (!binAsset || binAsset->buffer.empty())
    {
        Core::Logger::Error("FontProvider::CreateResource: 폰트 파일 에셋이 없거나 비어 있다");
        return nullptr;
    }

    auto fontRes = std::make_shared<FontResource>();
    if (!fontRes->Initialize(m_ftLibrary, std::move(binAsset)))
        return nullptr; // 원인은 FontResource가 남김

    return fontRes;
}

void FontProvider::ReleaseResource(std::shared_ptr<IResource> resource)
{
    // GPU 리소스가 아닌 CPU 메모리 객체이므로 스코프 이탈 시 자동 소멸
    return;
}