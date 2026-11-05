export module Contract.Render.Service.Descriptors:Brush;

import :Resource;
import Contract.Render.Assets;

export struct BrushDesc : public ResourceDesc
{
    ~BrushDesc() override = default;
    using ResourceDesc::ResourceDesc;

    Core::TypeID GetAssetTypeID() const override
    {
        return TextureAsset::StaticTypeID();
    }
};