export module Service.Render.Repository:Implementation;

import std;
import :ResourceTypes;
import Core.Handle;
import Contract.Common.Asset;
import Service.Render.Descriptors;
import Contract.Render.Interfaces;
import Delete.Service.AssetAsyncHelper;

export struct ResourceImplTag {}; // 내부 전용 Tag - 외부 Tag와 무관
using RawHandle = Core::IDHandle<ResourceImplTag>;

struct ResourceEntry
{
    std::size_t key{};
    std::shared_ptr<IResource> res{ nullptr };
    LoadState state{ LoadState::Pending };
};

struct CpuPendingRequest 
{ 
    RawHandle handle; 
    AssetRequestID requestId; 
};

struct GpuPendingRequest 
{ 
    RawHandle handle; 
    std::shared_ptr<Asset> asset; 
};

export class ResourceRepositoryImpl
{
public:
    ~ResourceRepositoryImpl() = default;
    ResourceRepositoryImpl(IResourceProvider* provider, IAssetAsyncLoader* asyncLoader);

    RawHandle Acquire(const ResourceDesc& desc);
    RawHandle AcquireFromAsset(const ResourceDesc& desc, std::shared_ptr<Asset> asset);

    bool Release(RawHandle handle);
    void ReleaseAll();

    void Update();

    [[nodiscard]] std::shared_ptr<IResource> GetIfReady(RawHandle handle) const;

private:
    std::pair<RawHandle, bool> FindOrRegister(std::size_t key);

    // ---- 파이프라인 단계별 처리 ----
    void ProcessAssetPending();
    void ProcessResourcePending();
    void ProcessLoading();

private:
    IResourceProvider* m_provider{ nullptr };
    IAssetAsyncLoader* m_asyncLoader{ nullptr };

    std::unordered_map<std::size_t, RawHandle> m_cache;
    Core::HandlePool<ResourceEntry, ResourceImplTag> m_loadedResources;

    std::vector<CpuPendingRequest> m_assetPending;
    std::vector<GpuPendingRequest> m_resourcePending;
    std::vector<RawHandle> m_loadingList;
};