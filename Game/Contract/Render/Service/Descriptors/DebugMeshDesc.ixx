export module Contract.Render.Service.Descriptors:DebugMesh;

import :Resource;
import Contract.Render.Assets;

export struct DebugMeshDesc : public ResourceDesc
{
    ~DebugMeshDesc() override = default;
    using ResourceDesc::ResourceDesc;

    Core::TypeID GetAssetTypeID() const override
    {
        return MeshAsset::StaticTypeID();
    }
};