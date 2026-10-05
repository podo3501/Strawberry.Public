export module Client.Render.Repository:Implementation;

import std;
import :ResourceTypes;
import Core.Handle;
import Core.Assert;
import Client.Asset.AssetData;
import Client.Render.Descriptors;
import Client.Render.IResource;
import Client.Render.Interfaces;
import Client.AssetAsyncHelper;
import Client.IAssetAsyncLoader;

struct ResourceImplTag {}; // 내부 전용 Tag - 외부 Tag와 무관

struct ResourceEntry
{
    std::size_t key{};
    std::shared_ptr<IResource> res{ nullptr };
    LoadState state{ LoadState::Pending };
};

export class ResourceRepositoryImpl
{
public:
    using RawHandle = Core::IDHandle<ResourceImplTag>;

    ResourceRepositoryImpl(IResourceProvider* provider, IAssetAsyncLoader* asyncLoader)
        : m_provider{ provider }
        , m_asyncLoader{ asyncLoader }
    {
    }

    ~ResourceRepositoryImpl() = default;

    RawHandle Acquire(const ResourceDesc& desc)
    {
        Core::Assert(desc.GetResourceID().GetType() == Core::ResourceIDType::Path);

        auto [handle, isNew] = FindOrRegister(desc.GetHash());
        if (!isNew)
            return handle;

        auto reqID = PushRequest(m_asyncLoader, desc.GetAssetTypeID(), desc.GetResourceID());
        m_assetPending.push_back({ handle, reqID });
        return handle;
    }

    RawHandle AcquireFromAsset(const ResourceDesc& desc, std::shared_ptr<AssetData> asset)
    {
        Core::Assert(desc.GetResourceID().GetType() != Core::ResourceIDType::Path);

        auto [handle, isNew] = FindOrRegister(desc.GetHash());
        if (!isNew)
            return handle;

        m_resourcePending.push_back({ handle, asset });
        return handle;
    }

    bool Release(RawHandle handle)
    {
        auto entry = m_loadedResources.Find(handle);
        if (!entry) return false;

        m_cache.erase(entry->key);
        std::erase(m_loadingList, handle);

        m_provider->ReleaseResource(std::move(entry->res));
        return m_loadedResources.Remove(handle);
    }

    void ReleaseAll()
    {
        m_loadedResources.Visit([this](RawHandle h, ResourceEntry&) {
            Release(h);
            });

        m_assetPending.clear();
        m_resourcePending.clear();

        m_loadingList.clear();
        m_cache.clear();
        m_loadedResources.Clear();
    }

    void Update()
    {
        ProcessAssetPending();
        ProcessResourcePending();
        ProcessLoading();
    }

    [[nodiscard]] std::shared_ptr<IResource> GetIfReady(RawHandle handle) const
    {
        auto entry = m_loadedResources.Find(handle);
        if (entry && entry->state == LoadState::Ready)
            return entry->res;

        return nullptr;
    }

private:
    struct CpuPendingRequest { RawHandle handle; AssetRequestID requestId; };
    struct GpuPendingRequest { RawHandle handle; std::shared_ptr<AssetData> asset; };

    std::pair<RawHandle, bool> FindOrRegister(std::size_t key)
    {
        auto it = m_cache.find(key);
        if (it != m_cache.end())
            return { it->second, false }; // 이미 존재

        ResourceEntry entry;
        entry.key = key;
        auto handle = m_loadedResources.Emplace(std::move(entry));
        m_cache[key] = handle;

        return { handle, true }; // 새로 등록됨
    }

    // ---- 파이프라인 단계별 처리 ----

    void ProcessAssetPending()
    {
        if (m_assetPending.empty())
            return;

        for (auto it = m_assetPending.begin(); it != m_assetPending.end();)
        {
            auto& req = *it;
            auto asset = m_asyncLoader->TakeResult(req.requestId);
            if (!asset)
            {
                ++it;
                continue;
            }

            GpuPendingRequest gpuReq;
            gpuReq.handle = req.handle;
            gpuReq.asset = asset;
            m_resourcePending.push_back(std::move(gpuReq));

            it = m_assetPending.erase(it);
        }
    }

    void ProcessResourcePending()
    {
        for (auto& work : m_resourcePending)
        {
            auto entry = m_loadedResources.Find(work.handle);
            if (!entry) continue;
            //if (entry->state != LoadState::Pending) continue; // 중복으로 들어온 경우 이미 Loading/Ready 라면 처리 안 함.

            auto res = m_provider->CreateResource(work.asset);
            if (!res)
            {
                Core::Assert(false); // 로딩하다가 실패함.
                entry->state = LoadState::Failed;
                continue;
            }

            entry->res = std::move(res);
            entry->state = LoadState::ResourceLoading;
            m_loadingList.push_back(work.handle);
        }

        m_resourcePending.clear();
    }

    void ProcessLoading()
    {
        for (auto it = m_loadingList.begin(); it != m_loadingList.end(); )
        {
            auto entry = m_loadedResources.Find(*it);
            if (!entry || !entry->res)
            {
                it = m_loadingList.erase(it);
                continue;
            }

            if (entry->res->IsReady())
            {
                entry->state = LoadState::Ready;
                it = m_loadingList.erase(it);
            }
            else
                ++it;
        }
    }

private:
    IResourceProvider* m_provider{ nullptr };
    IAssetAsyncLoader* m_asyncLoader{ nullptr };

    std::unordered_map<std::size_t, RawHandle> m_cache;
    Core::HandlePool<ResourceEntry, ResourceImplTag> m_loadedResources;

    std::vector<CpuPendingRequest> m_assetPending;
    std::vector<GpuPendingRequest> m_resourcePending;
    std::vector<RawHandle> m_loadingList;
};