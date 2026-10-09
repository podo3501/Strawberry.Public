export module Service.Render.Descriptors:DebugMaterial;

import :Resource;
import Core.Utils;
import Contract.Asset.AssetData;
import Contract.Asset.Data;

export struct DebugMaterialDesc : public ResourceDesc
{
private:
    DebugMaterialType type;

public:
    virtual Core::TypeID GetAssetTypeID() const override
    {
        return AssetData::StaticTypeID(); // 현재 Debug Material은 Asset을 사용하지 않음.
    }

    size_t GetHash() const { return Core::HashOf(ResourceDesc::GetHash(), type); }
    DebugMaterialType GetType() const { return type; }

protected:
    DebugMaterialDesc(Core::ResourceID resID, DebugMaterialType type) :
        ResourceDesc{ resID },
        type{ type }
    {
    }
};