export module Service.Render.Descriptors:DebugMesh;

import :Resource;
import Contract.Render.Assets;

export struct DebugMeshDesc : public ResourceDesc
{
    using ResourceDesc::ResourceDesc;

    virtual Core::TypeID GetAssetTypeID() const override
    {
        return MeshAsset::StaticTypeID();
    }
};