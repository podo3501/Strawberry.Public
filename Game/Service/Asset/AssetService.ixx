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
    
    bool Initialize(size_t threadCount = 0) override;
    bool IsIdle() const override;

    IAssetAsyncLoader* GetAsyncLoader() noexcept override;

private:
    
    bool StartWorkers(size_t threadCount);
    void StopWorkers();
    void ThreadLoop();

private:
    // 선언 순서가 곧 수명 순서다: 포인터를 쥔 쪽(m_repository의 로더 -> m_metaRegistry -> m_asyncLoader)이 가리키는 쪽보다 먼저 해제되도록 늦게 선언한다
    AssetAsyncLoader m_asyncLoader;
    AssetMetaRegistry m_metaRegistry;
    AssetRepository m_repository;

    std::vector<std::thread> m_threads;
    std::atomic<int> m_activeJobs{ 0 };
};