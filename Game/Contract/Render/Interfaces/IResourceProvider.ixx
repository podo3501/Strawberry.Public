export module Client.Render.Interfaces:IResourceProvider;

import std;
import :IResource;
import Client.Asset.AssetData;

export struct IResourceProvider
{
    virtual ~IResourceProvider() = default;

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) = 0;
    virtual void ReleaseResource(std::shared_ptr<IResource> res) = 0;
};