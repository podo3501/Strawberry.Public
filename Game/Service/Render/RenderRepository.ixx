export module Service.Render:Repository;

import std;
import Contract.Render.Service;
import Service.Render.Repository;
import Contract.Render.Assets;

//이 클래스는 지금 단순 포워딩 함수이지만 정책 코드가 안 들어가서 그렇다. 예를들면 엑셀에서 읽어와서 넣는다던가..
export class RenderRepository : public IRenderRepository
{
public:
    ~RenderRepository() override = default;
    RenderRepository(RepositoryContainer& repositories);

    FontHandle LoadFont(const FontDesc& desc) override;
    bool ReleaseFont(FontHandle fh) override;

    MeshHandle LoadMesh(const MeshDesc& desc, std::shared_ptr<MeshAsset> asset = nullptr) override;
    bool ReleaseMesh(MeshHandle mh) override;

    DebugMeshHandle LoadDebugMesh(const DebugMeshDesc& desc, std::shared_ptr<MeshAsset> asset = nullptr) override;

    MaterialHandle LoadMaterial(const MaterialDesc& desc) override;
    bool ReleaseMaterial(MaterialHandle mh) override;

    DebugMaterialHandle LoadDebugMaterial(const DebugMaterialDesc& desc) override;
    bool ReleaseDebugMaterial(DebugMaterialHandle dmh) override;

    BrushHandle LoadBrush(const BrushDesc& desc) override;
    bool ReleaseBrush(BrushHandle bh) override;

    EnvironmentHandle LoadEnvironment(const EnvironmentDesc& desc) override;
    bool ReleaseEnvironment(EnvironmentHandle eh) override;

    void ReleaseAll() override;

    void Update();

private:
    RepositoryContainer& m_repositories;
};