export module Service.AssetMetaRegistry;

import std;
import Core.Type;
import Service.AssetAsyncHelper;
import Contract.Common.Asset;
import Contract.Asset.Service;

export class AssetMetaRegistry : public IAssetMetaRegistry
{
public:
    ~AssetMetaRegistry() override = default;
    AssetMetaRegistry(IAssetAsyncLoader* loader, IAssetStorage* storage);

	bool Initialize();   // 확장자별 메타 종류를 등록한다
	bool Scan();         // 저장소의 .meta 파일을 모두 읽는다. 하나라도 못 읽으면 false(못 읽은 파일마다 Error를 남기고 나머지는 계속 읽는다). 부를 때마다 이전 결과는 비운다. 읽는 로더가 등록된 뒤, 작업자 스레드가 도는 중에 불러야 한다
	std::shared_ptr<Asset> GetMeta(const Core::ResourceID& resID) const override;

private:
    bool RegisterMetaType(std::string_view originalExt, Core::TypeID metaTypeID);

    IAssetAsyncLoader* m_asyncLoader{ nullptr };
    IAssetStorage* m_storage{ nullptr };
    std::unordered_map<std::string, Core::TypeID> m_extToMetaType;
    std::unordered_map<Core::ResourceID, std::shared_ptr<Asset>> m_metaAssets;
};