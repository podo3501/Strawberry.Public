export module DxRender.Provider:TextureLoadRequest;

import std;
import Contract.Asset.Data;
import DxRender.Resource;

export struct TextureLoadRequest
{
    std::shared_ptr<TextureResource> resource;
    std::shared_ptr<TextureAsset> asset;

    std::size_t estimatedBytes{ 0 };
};