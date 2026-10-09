export module Service.Render.Descriptors:Brush;

import :Resource;
import Contract.Asset.Data;

export struct BrushDesc : public ResourceDesc
{
    using ResourceDesc::ResourceDesc;

    virtual Core::TypeID GetAssetTypeID() const override
    {
        return TextureAsset::StaticTypeID();
    }
};