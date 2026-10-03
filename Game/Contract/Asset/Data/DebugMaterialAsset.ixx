export module Client.Asset.Data:DebugMaterialAsset;

import :DebugMaterialType;
import Client.Asset.AssetData;
import Core.TypeHierarchy;

export struct DebugMaterialAsset : public Core::TypeNode<DebugMaterialAsset, AssetData>
{
    virtual ~DebugMaterialAsset() = default;
    DebugMaterialType type;
};