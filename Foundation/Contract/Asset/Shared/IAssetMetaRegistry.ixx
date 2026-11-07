export module Contract.Asset.Shared:IAssetMetaRegistry;

import std;
import Core.ResourceID;
import Contract.Common.Asset;

// 로더(백엔드)가 에셋의 .meta 정보를 조회하는 통로. 레지스트리는 AssetService가 만들고 소유하며, 로더에는 이 인터페이스로만 보인다.
export struct IAssetMetaRegistry
{
	virtual ~IAssetMetaRegistry() = default;
	virtual std::shared_ptr<Asset> GetMeta(const Core::ResourceID& resID) const = 0;   // 없으면 nullptr
};