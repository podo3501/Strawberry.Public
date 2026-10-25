export module Contract.Asset.Shared:IAssetMetaRegistry;

import std;
import Core.ResourceID;
import Contract.Common.Asset;
import Contract.Common.IAssetAsyncLoader;

export struct IAssetMetaRegistry
{
	virtual ~IAssetMetaRegistry() = default;
	virtual bool Initialize() = 0;
	virtual bool Scan(const std::filesystem::path& resPath) =0;
	virtual std::shared_ptr<Asset> GetMeta(const Core::ResourceID& resID) const = 0;
};

export std::unique_ptr<IAssetMetaRegistry> CreateAssetMetaRegistry(IAssetAsyncLoader* loader);