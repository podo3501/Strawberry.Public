export module Contract.Asset.Data:DebugMaterialAsset;

import :DebugMaterialType;
import Contract.Asset.AssetData;
import Core.TypeHierarchy;

export struct DebugMaterialAsset : public Core::TypeNode<DebugMaterialAsset, AssetData>
{
    virtual ~DebugMaterialAsset() = default;
    DebugMaterialType type;
};