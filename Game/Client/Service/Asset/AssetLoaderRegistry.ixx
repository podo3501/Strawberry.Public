export module Service.Asset:AssetLoaderRegistry;

import std;
import :AssetRepository;
import Contract.Asset;
import Service.AssetMetaRegistry;

export class AssetLoaderRegistry
{
public:
    ~AssetLoaderRegistry() = default;
    explicit AssetLoaderRegistry(AssetRepository& repository) noexcept;
    bool RegisterDefaultLoaders(IAssetMetaRegistry* metaRegistry);

private:
    template <typename AssetType>
    bool RegisterLoader(std::string_view ext, std::unique_ptr<IAssetLoader>&& loader)
    {
        return m_repository.RegisterLoader(
            AssetLoaderDesc::Make<AssetType>(ext, std::move(loader))
        );
    }

private:
    AssetRepository& m_repository;
};