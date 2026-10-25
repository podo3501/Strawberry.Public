export module Service.Asset;

import std;
import :AssetRepository;
import :AsyncLoader;
import Contract.Asset.Service;
import Contract.Asset.Backend;
import Service.AssetMetaRegistry;
import Contract.Common.IAssetAsyncLoader;

export class AssetService : public IAssetService
{
public:
    ~AssetService() override;
    AssetService() = delete;
    explicit AssetService(IAssetStorage* assetStorage) noexcept;
    
    bool Initialize(IAssetMetaRegistry* metaRegistry, size_t threadCount = 0) override;
    bool IsIdle() const override;

    IAssetAsyncLoader* GetAsyncLoader() noexcept override;

private:
    
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