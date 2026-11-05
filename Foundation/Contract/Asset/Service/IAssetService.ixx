export module Contract.Asset.Service:IAssetService;

import std;
import Contract.Asset.Shared;
import Contract.Common.IAssetAsyncLoader;

export struct IAssetService
{
	virtual ~IAssetService() = default;

	virtual bool Initialize(IAssetMetaRegistry* metaRegistry, size_t threadCount = 0) = 0;
	virtual bool IsIdle() const = 0;

	virtual IAssetAsyncLoader* GetAsyncLoader() noexcept = 0;
};

export std::unique_ptr<IAssetService> CreateAssetService(IAssetStorage* assetStorage);