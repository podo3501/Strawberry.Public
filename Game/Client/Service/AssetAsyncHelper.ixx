export module Service.AssetAsyncHelper;

import std;
import Core.ResourceID;
import Core.TypeHierarchy;
import Service.IAssetAsyncLoader;

export template<typename T>
    AssetRequest MakeRequest(Core::ResourceID resID)
{
    return
    {
        .resID = std::move(resID),
        .type = Core::GetTypeID<T>()
    };
}

export template<typename T>
    AssetRequest MakeRequest(std::string_view path)
{
    return
    {
        .resID = Core::ResourceID::MakePath(path),
        .type = Core::GetTypeID<T>()
    };
}

export inline AssetRequest MakeRequest(Core::TypeID typeID, Core::ResourceID resID)
{
    return
    {
        .resID = std::move(resID),
        .type = typeID
    };
}

export inline AssetRequest MakeRequest(Core::TypeID typeID, const std::string& path)
{
    return MakeRequest(typeID, Core::ResourceID::MakePath(path));
}

export template<typename T>
    AssetRequestID PushRequest(IAssetAsyncLoader* asyncLoader, Core::ResourceID resID)
{
    return asyncLoader->PushRequest(MakeRequest<T>(resID));
}

export template<typename T>
    AssetRequestID PushRequest(IAssetAsyncLoader* asyncLoader, std::string_view path)
{
    return PushRequest<T>(asyncLoader, Core::ResourceID::MakePath(path));
}

export inline AssetRequestID PushRequest(IAssetAsyncLoader* asyncLoader, Core::TypeID typeID, Core::ResourceID resID)
{
    return asyncLoader->PushRequest(MakeRequest(typeID, std::move(resID)));
}

export inline AssetRequestID PushRequest(IAssetAsyncLoader* asyncLoader, Core::TypeID typeID, std::string_view path)
{
    return PushRequest(asyncLoader, typeID, Core::ResourceID::MakePath(path));
}

export template<typename T>
    std::shared_ptr<T> TakeResult(IAssetAsyncLoader* asyncLoader, AssetRequestID id)
{
    AssetPtr ptr = asyncLoader->TakeResult(id);
    return std::static_pointer_cast<T>(ptr);
}

export template <typename T>
    std::shared_ptr<T> Wait(IAssetAsyncLoader* asyncLoader, AssetRequestID id)
{
    return Core::Cast<T>(asyncLoader->Wait(id));
}

export std::vector<AssetRequestID> PushRequests(
    IAssetAsyncLoader* asyncLoader,
    std::span<const AssetRequest> requests)
{
    std::vector<AssetRequestID> ids;
    ids.reserve(requests.size());

    for (const AssetRequest& request : requests)
        ids.emplace_back(asyncLoader->PushRequest(request));

    return ids;
}

export std::vector<AssetPtr> WaitAll(
    IAssetAsyncLoader* asyncLoader,
    std::span<const AssetRequestID> ids)
{
    std::vector<AssetPtr> results;
    results.reserve(ids.size());

    for (AssetRequestID id : ids)
        results.push_back(asyncLoader->Wait(id));

    return results;
}