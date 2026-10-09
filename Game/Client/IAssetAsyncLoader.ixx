export module Service.IAssetAsyncLoader;

export import Service.AssetAsyncTypes;

export struct IAssetAsyncLoader
{
	virtual ~IAssetAsyncLoader() = default;

	virtual AssetRequestID PushRequest(AssetRequest req) = 0;
	virtual AssetPtr TakeResult(AssetRequestID id) = 0;
	virtual AssetPtr Wait(AssetRequestID id) = 0;
};
