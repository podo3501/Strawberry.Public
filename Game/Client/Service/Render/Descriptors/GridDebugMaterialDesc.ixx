export module Service.Render.Descriptors:GridDebugMaterial;

import :DebugMaterial;

export struct GridDebugMaterialDesc : public DebugMaterialDesc
{
    explicit GridDebugMaterialDesc(Core::ResourceID resID) :
        DebugMaterialDesc{ resID, DebugMaterialType::Grid }
    {
    }
};