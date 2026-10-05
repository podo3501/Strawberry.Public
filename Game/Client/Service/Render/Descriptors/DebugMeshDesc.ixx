export module Client.Render.Descriptors:DebugMesh;

import :Resource;
import Client.Asset.Data;

export struct DebugMeshDesc : public ResourceDesc
{
    using ResourceDesc::ResourceDesc;

    virtual Core::TypeID GetAssetTypeID() const override
    {
        return MeshAsset::StaticTypeID();
    }
};