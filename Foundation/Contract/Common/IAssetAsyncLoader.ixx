export module Contract.Common.IAssetAsyncLoader;

import std;
export import Contract.Common.AssetAsyncTypes;

export struct IAssetAsyncLoader
{
	virtual ~IAssetAsyncLoader() = default;

	virtual AssetRequestID PushRequest(AssetRequest req) = 0;
	// 값 없음은 아직 끝나지 않음, 값이 nullptr이면 로딩 실패다. 결과는 한 번만 꺼낼 수 있다.
	virtual std::optional<AssetPtr> TakeResult(AssetRequestID id) = 0;
	virtual AssetPtr Wait(AssetRequestID id) = 0;
};
