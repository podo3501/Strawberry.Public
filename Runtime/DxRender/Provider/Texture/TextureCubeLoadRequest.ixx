export module DxRender.Provider:TextureCubeLoadRequest;

import std;
import Contract.Asset.Data;
import DxRender.Resource;

export struct TextureCubeLoadRequest
{
    std::shared_ptr<TextureCubeResource> resource;
    std::shared_ptr<TextureCubeAsset> asset;

    std::size_t estimatedBytes{ 0 };
};