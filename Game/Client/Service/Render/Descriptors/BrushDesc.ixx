export module Client.Render.Descriptors:Brush;

import :Resource;
import Client.Asset.Data;

export struct BrushDesc : public ResourceDesc
{
    using ResourceDesc::ResourceDesc;

    virtual Core::TypeID GetAssetTypeID() const override
    {
        return TextureAsset::StaticTypeID();
    }
};