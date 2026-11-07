module Service.Asset;

import :AssetLoaderRegistry;
import Core.Utils;

AssetService::~AssetService() { StopWorkers(); }
AssetService::AssetService(IAssetStorage* assetStorage) noexcept
	: m_metaRegistry{ &m_asyncLoader, assetStorage }
	, m_repository{ assetStorage }
{}

bool AssetService::Initialize(size_t threadCount)
{
    if (threadCount == 0)
    {
        threadCount = std::min<size_t>(
            std::max<size_t>(1, std::thread::hardware_concurrency() - 1),
            8);
    }

    if (!StartWorkers(threadCount))
        return false;

    // 레지스트리와 로더 등록이 서로를 필요로 하므로 이 순서로 한다: 메타 종류 등록 -> 로더 등록 -> .meta 읽기(등록된 로더와 작업자 스레드가 필요하다)
    if (!m_metaRegistry.Initialize())
        return false;

    AssetLoaderRegistry loaderRegistry(m_repository);
    if (!loaderRegistry.RegisterDefaultLoaders(&m_metaRegistry))
        return false;

    return m_metaRegistry.Scan();
}

bool AssetService::IsIdle() const
{
    if (m_threads.empty())
        return true;

    return m_activeJobs.load(std::memory_order_acquire) == 0
        && !m_asyncLoader.HasPendingWork();
}

IAssetAsyncLoader* AssetService::GetAsyncLoader() noexcept
{
    return &m_asyncLoader;
}

bool AssetService::StartWorkers(size_t threadCount)
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

void AssetService::StopWorkers()
{
    if (m_threads.empty())
        return;

    m_asyncLoader.Shutdown();
            
    for (auto& t : m_threads)
    {
        if (t.joinable())
            t.join();
    }

    m_threads.clear();
}

void AssetService::ThreadLoop()   
{
    while (true)
    {
        AssetRequestID id;
        if (!m_asyncLoader.WaitPopPending(id)) // 작업이 있으면 id로 작업을 가져옴
            break;

        ++m_activeJobs;

        auto reqOpt = m_asyncLoader.TakeRequest(id); // request 데이터 가져오기
        if (reqOpt)
        {
            AssetPtr result = m_repository.Load(reqOpt->type, reqOpt->resID);
            m_asyncLoader.PushResult(id, std::move(result)); // 결과 저장
        }

        --m_activeJobs;
    }
}

std::unique_ptr<IAssetService> CreateAssetService(IAssetStorage* assetStorage)
{
	if (!assetStorage)
		return nullptr;

    return std::make_unique<AssetService>(assetStorage);
}