export module DxRender.Provider:MeshLoadRequest;

import std;
import Contract.Asset.Data;
import DxRender.Resource;

export struct MeshLoadRequest
{
    std::shared_ptr<MeshResource> resource;
    std::shared_ptr<MeshAsset> asset;

    std::size_t vbBytes{ 0 };
    std::size_t ibBytes{ 0 };
    std::size_t estimatedBytes{ 0 };
};