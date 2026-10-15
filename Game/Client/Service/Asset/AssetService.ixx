export module Service.Asset:AssetService;

import std;
import :AssetRepository;
import :AsyncLoader;
import Contract.Asset;
import Service.AssetMetaRegistry;
import Service.IAssetAsyncLoader;

export class AssetService
{
public:
    ~AssetService();
    AssetService() = delete;
    static std::unique_ptr<AssetService> Create(IAssetStorage* assetStorage) noexcept;

    bool Initialize(IAssetMetaRegistry* metaRegistry, size_t threadCount = 0);
    bool IsIdle() const;

    IAssetAsyncLoader* GetAsyncLoader() noexcept;

private:
    explicit AssetService(IAssetStorage* assetStorage) noexcept;
    bool StartWorkers(size_t threadCount);
    void StopWorkers();
    void ThreadLoop();

private:
    AssetRepository m_repository;
    AssetAsyncLoader m_asyncLoader;
    IAssetMetaRegistry* m_metaRegistry{ nullptr };

    std::vector<std::thread> m_threads;
    std::atomic<int> m_activeJobs{ 0 };
};