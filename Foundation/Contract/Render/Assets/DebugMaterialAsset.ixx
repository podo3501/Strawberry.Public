export module Contract.Render.Assets:DebugMaterialAsset;

import :DebugMaterialType;
import Contract.Common.Asset;
import Core.TypeHierarchy;

export struct DebugMaterialAsset : public Core::TypeNode<DebugMaterialAsset, Asset>
{
    virtual ~DebugMaterialAsset() = default;
    DebugMaterialType type;
};