export module Client.Render:Repository;

import std;
import Client.Render.ResourceHandles;
import Client.Render.Descriptors;
import Client.Render.Repository;
import Client.Asset.Data;
import Core.ResourceID;
import Core.Assert;

//이 클래스는 지금 단순 포워딩 함수이지만 정책 코드가 안 들어가서 그렇다. 예를들면 엑셀에서 읽어와서 넣는다던가..
export class RenderRepository
{
public:
    ~RenderRepository() = default;

    RenderRepository(RepositoryContainer& repositories)
        : m_repositories{ repositories }
    {
    }

    FontHandle LoadFont(const FontDesc& desc)
    {
        return m_repositories.Acquire<FontRepository>(desc);
    }

    bool ReleaseFont(FontHandle fh)
    {
        return m_repositories.Release<FontRepository>(fh);
    }

    MeshHandle LoadMesh(const MeshDesc& desc, std::shared_ptr<MeshAsset> asset = nullptr)
    {
        if (desc.GetResourceID().GetType() != Core::ResourceIDType::Path)
        {
            Assert(asset); // runtime/builtin 인데 asset이 없으면 정상적이지 않음
            return m_repositories.AcquireFromAsset<MeshRepository>(desc, asset);
        }

        return m_repositories.Acquire<MeshRepository>(desc);
    }

    bool ReleaseMesh(MeshHandle mh)
    {
        return m_repositories.Release<MeshRepository>(mh);
    }

    DebugMeshHandle LoadDebugMesh(const DebugMeshDesc& desc, std::shared_ptr<MeshAsset> asset = nullptr)
    {
        if (desc.GetResourceID().GetType() != Core::ResourceIDType::Path)
        {
            Assert(asset); // runtime/builtin 인데 asset이 없으면 정상적이지 않음
            return m_repositories.AcquireFromAsset<DebugMeshRepository>(desc, asset);
        }

        return m_repositories.Acquire<DebugMeshRepository>(desc);
    }

    MaterialHandle LoadMaterial(const MaterialDesc& desc)
    {
        return m_repositories.Acquire<MaterialRepository>(desc);
    }

    bool ReleaseMaterial(MaterialHandle mh)
    {
        return m_repositories.Release<MaterialRepository>(mh);
    }

    DebugMaterialHandle LoadDebugMaterial(const DebugMaterialDesc& desc)
    {
        auto asset = std::make_shared<DebugMaterialAsset>();
        asset->type = desc.GetType();

        return m_repositories.AcquireFromAsset<DebugMaterialRepository>(desc, std::move(asset));
    }

    bool ReleaseDebugMaterial(DebugMaterialHandle dmh)
    {
        return m_repositories.Release<DebugMaterialRepository>(dmh);
    }

    BrushHandle LoadBrush(const BrushDesc& desc)
    {
        return m_repositories.Acquire<BrushRepository>(desc);
    }

    bool ReleaseBrush(BrushHandle bh)
    {
        return m_repositories.Release<BrushRepository>(bh);
    }

    EnvironmentHandle LoadEnvironment(const EnvironmentDesc& desc)
    {
        return m_repositories.Acquire<EnvironmentRepository>(desc);
    }

    bool ReleaseEnvironment(EnvironmentHandle eh)
    {
        return m_repositories.Release<EnvironmentRepository>(eh);
    }

    void Update()
    {
        m_repositories.UpdateAll();
    }

    void ReleaseAll()
    {
        m_repositories.ReleaseAll();
    }

private:
    RepositoryContainer& m_repositories;
};