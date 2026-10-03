export module Runtime.Render.Provider:TextureCubeLoadRequest;

import std;
import Client.Asset.Data;
import Runtime.Render.Resource;

export struct TextureCubeLoadRequest
{
    std::shared_ptr<TextureCubeResource> resource;
    std::shared_ptr<TextureCubeAsset> asset;

    std::size_t estimatedBytes{ 0 };
};