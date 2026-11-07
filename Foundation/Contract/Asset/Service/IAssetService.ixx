export module Contract.Asset.Service:IAssetService;

import std;
import Contract.Asset.Shared;
import Contract.Common.IAssetAsyncLoader;

export struct IAssetService
{
	virtual ~IAssetService() = default;

	// 작업자 스레드를 시작하고 기본 로더를 등록한 뒤, 저장소의 .meta 파일을 모두 읽는다. 어느 단계든 실패하면 false
	virtual bool Initialize(size_t threadCount = 0) = 0;
	virtual bool IsIdle() const = 0;

	virtual IAssetAsyncLoader* GetAsyncLoader() noexcept = 0;
};

export std::unique_ptr<IAssetService> CreateAssetService(IAssetStorage* assetStorage);