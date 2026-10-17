module DxRender.Provider;

import std;
import :Font;
import DxRender.Resource;
import Contract.Asset.Data;

std::shared_ptr<IResource> FontProvider::CreateResource(std::shared_ptr<AssetData> asset)
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

void FontProvider::ReleaseResource(std::shared_ptr<IResource> resource)
{
    // GPU 리소스가 아닌 CPU 메모리 객체이므로 스코프 이탈 시 자동 소멸
    return;
}