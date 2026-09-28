export module Client.Asset.Service;

import std;
import :AssetRepository;
import :AssetLoaderRegistry;
import :AsyncLoader;
import Core.Utils;
import Client.Asset.Contract;
import Client.AssetMetaRegistry;
import Client.IAssetAsyncLoader;

namespace Client
{
    export class AssetService
    {
    public:
        ~AssetService()
        {
            StopWorkers();
        }

        AssetService() = delete;

        static std::unique_ptr<AssetService> Create(IAssetStorage* assetStorage) noexcept
        {
            if (!assetStorage)
                return nullptr;

            return std::unique_ptr<AssetService>(new AssetService(assetStorage));
        }

        bool Initialize(IAssetMetaRegistry* metaRegistry, size_t threadCount = 0)
        {
            m_metaRegistry = metaRegistry;

            if (threadCount == 0)
            {
                threadCount = std::min<size_t>(
                    std::max<size_t>(1, std::thread::hardware_concurrency() - 1),
                    8);
            }

            if (!StartWorkers(threadCount))
                return false;

            AssetLoaderRegistry loaderRegistry(*m_repository);
            if (!loaderRegistry.RegisterDefaultLoaders(metaRegistry))
                return false;

            return true;
        }

        bool IsIdle() const
        {
            if (m_threads.empty())
                return true;

            return m_activeJobs.load(std::memory_order_acquire) == 0
                && !m_asyncLoader->HasPendingWork();
        }

        IAssetAsyncLoader* GetAsyncLoader() noexcept
        {
            return m_asyncLoader.get();
        }

    private:
        explicit AssetService(IAssetStorage* assetStorage) noexcept
            : m_repository{ std::make_unique<AssetRepository>(assetStorage) }
            , m_asyncLoader{ std::make_unique<AssetAsyncLoader>() }
        {
        }

        bool StartWorkers(size_t threadCount)
        {
            if (!m_threads.empty())
                return false;

            m_threads.reserve(threadCount);
            for (size_t i = 0; i < threadCount; ++i)
            {
                m_threads.emplace_back([this]() {
                    ThreadLoop();
                    });
            }

            return true;
        }

        void StopWorkers()
        {
            if (m_threads.empty())
                return;

            if (m_asyncLoader)
                m_asyncLoader->Shutdown();

            for (auto& t : m_threads)
            {
                if (t.joinable())
                    t.join();
            }

            m_threads.clear();
        }

        void ThreadLoop()
        {
            while (true)
            {
                AssetRequestID id;
                if (!m_asyncLoader->WaitPopPending(id)) // 작업이 있으면 id로 작업을 가져옴
                    break;

                ++m_activeJobs;

                auto reqOpt = m_asyncLoader->TakeRequest(id); // request 데이터 가져오기
                if (reqOpt)
                {
                    AssetPtr result = m_repository->Load(reqOpt->type, reqOpt->resID);
                    m_asyncLoader->PushResult(id, std::move(result)); // 결과 저장
                }

                --m_activeJobs;
            }
        }

    private:
        std::unique_ptr<AssetRepository> m_repository{ nullptr };
        std::unique_ptr<AssetAsyncLoader> m_asyncLoader{ nullptr };
        IAssetMetaRegistry* m_metaRegistry{ nullptr };

        std::vector<std::thread> m_threads;
        std::atomic<int> m_activeJobs{ 0 };
    };
}