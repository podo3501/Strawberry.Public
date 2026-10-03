export module Runtime.Render.Provider:TextureLoadRequest;

import std;
import Client.Asset.Data;
import Runtime.Render.Resource;

export struct TextureLoadRequest
{
    std::shared_ptr<TextureResource> resource;
    std::shared_ptr<TextureAsset> asset;

    std::size_t estimatedBytes{ 0 };
};