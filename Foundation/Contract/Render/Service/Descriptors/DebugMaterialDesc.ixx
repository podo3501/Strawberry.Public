export module Contract.Render.Service.Descriptors:DebugMaterial;

import :Resource;
import Core.Utils;
import Contract.Common.Asset;
import Contract.Render.Assets;

export struct DebugMaterialDesc : public ResourceDesc
{
private:
    DebugMaterialType type;

public:
    ~DebugMaterialDesc() override = default;

    Core::TypeID GetAssetTypeID() const override
    {
        return Asset::StaticTypeID(); // 현재 Debug Material은 Asset을 사용하지 않음.
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