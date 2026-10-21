export module Service.Render:Repository;

import std;
import Service.Render.Handles;
import Service.Render.Descriptors;
import Service.Render.Repository;
import Contract.Render.Assets;

//이 클래스는 지금 단순 포워딩 함수이지만 정책 코드가 안 들어가서 그렇다. 예를들면 엑셀에서 읽어와서 넣는다던가..
export class RenderRepository
{
public:
    ~RenderRepository() = default;
    RenderRepository(RepositoryContainer& repositories);

    FontHandle LoadFont(const FontDesc& desc);
    bool ReleaseFont(FontHandle fh);

    MeshHandle LoadMesh(const MeshDesc& desc, std::shared_ptr<MeshAsset> asset = nullptr);
    bool ReleaseMesh(MeshHandle mh);

    DebugMeshHandle LoadDebugMesh(const DebugMeshDesc& desc, std::shared_ptr<MeshAsset> asset = nullptr);

    MaterialHandle LoadMaterial(const MaterialDesc& desc);
    bool ReleaseMaterial(MaterialHandle mh);

    DebugMaterialHandle LoadDebugMaterial(const DebugMaterialDesc& desc);
    bool ReleaseDebugMaterial(DebugMaterialHandle dmh);

    BrushHandle LoadBrush(const BrushDesc& desc);
    bool ReleaseBrush(BrushHandle bh);

    EnvironmentHandle LoadEnvironment(const EnvironmentDesc& desc);
    bool ReleaseEnvironment(EnvironmentHandle eh);

    void Update();
    void ReleaseAll();

private:
    RepositoryContainer& m_repositories;
};