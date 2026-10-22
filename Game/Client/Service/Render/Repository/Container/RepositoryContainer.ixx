export module Service.Render.Repository:RepositoryContainer;

import std;
import :Interface;
import :RepositoryTypes;
import :ResourceRepositories;
import :TypeTraits;
import Delete.Service.AssetAsyncHelper;
import Core.Utils;
import Core.Assert;
import Contract.Render.Backend;

export class RepositoryContainer
{
public:
    RepositoryContainer(IRenderBackend& backend, IAssetAsyncLoader* asyncLoader)
        : m_repositories(Core::EnumSize<RepositoryType>)
    {
		Emplace<FontRepository>(backend.GetProvider(ProviderType::Font), asyncLoader);
		Emplace<MeshRepository>(backend.GetProvider(ProviderType::Mesh), asyncLoader);
		Emplace<MaterialRepository>(backend.GetProvider(ProviderType::Material), asyncLoader);
		Emplace<DebugMeshRepository>(backend.GetProvider(ProviderType::Mesh), asyncLoader);
		Emplace<DebugMaterialRepository>(backend.GetProvider(ProviderType::DebugMaterial), asyncLoader);
		Emplace<BrushRepository>(backend.GetProvider(ProviderType::Brush), asyncLoader);
		Emplace<EnvironmentRepository>(backend.GetProvider(ProviderType::Environment), asyncLoader);
    }

    ~RepositoryContainer() = default;

    RepositoryContainer(const RepositoryContainer&) = delete;
    RepositoryContainer& operator=(const RepositoryContainer&) = delete;

    RepositoryContainer(RepositoryContainer&&) noexcept = default;
    RepositoryContainer& operator=(RepositoryContainer&&) noexcept = default;

    template <typename TRepository>
    [[nodiscard]] TRepository& Get() const
    {
        constexpr auto type = RepositoryTypeOf<TRepository>::value;
        auto* repository = static_cast<TRepository*>(
            m_repositories[Core::ToIndex(type)].get());

        Core::Assert(repository);
        return *repository;
    }

    void UpdateAll()
    {
        for (auto& repo : m_repositories)
        {
            if (repo)
                repo->Update();
        }
    }

    void ReleaseAll()
    {
        for (auto& repo : m_repositories)
        {
            if (repo)
                repo->ReleaseAll();
        }
    }

    // 매 프레임 부르지 않는 헬퍼 함수.

    template <typename TRepository, typename... Args>
    auto Acquire(Args&&... args)
    {
        return Get<TRepository>().Acquire(std::forward<Args>(args)...);
    }

    template <typename TRepository, typename... Args>
    auto AcquireFromAsset(Args&&... args)
    {
        return Get<TRepository>().AcquireFromAsset(std::forward<Args>(args)...);
    }

    template <typename TRepository, typename THandle>
    bool Release(THandle handle)
    {
        return Get<TRepository>().Release(handle);
    }

private:
	template <typename TRepository, typename... Args>
	TRepository& Emplace(Args&&... args)
	{
		static_assert(std::is_base_of_v<IResourceRepository, TRepository>);
		constexpr auto type = RepositoryTypeOf<TRepository>::value; // 인스턴스화 시점에 특수화만 보이면 OK

		auto repository = std::make_unique<TRepository>(std::forward<Args>(args)...);
		TRepository* ptr = repository.get();
		m_repositories[Core::ToIndex(type)] = std::move(repository);

		return *ptr;
	}

    std::vector<std::unique_ptr<IResourceRepository>> m_repositories;
};