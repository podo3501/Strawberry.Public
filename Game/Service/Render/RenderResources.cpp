module Service.Render;

import :Repository;
import Service.Render.Repository;
import Core.ResourceID;
import Core.Assert;
import Contract.Render.Assets;

RenderResources::RenderResources(RepositoryContainer& repositories)
    : m_repositories{ repositories }
{}

FontHandle RenderResources::LoadFont(const FontDesc& desc)
{
    return m_repositories.Acquire<FontRepository>(desc);
}

bool RenderResources::ReleaseFont(FontHandle fh)
{
    return m_repositories.Release<FontRepository>(fh);
}

MeshHandle RenderResources::LoadMesh(const MeshDesc& desc, std::shared_ptr<MeshAsset> asset)
{
    if (desc.GetResourceID().GetType() != Core::ResourceIDType::Path)
    {
        Core::Assert(asset); // runtime/builtin 인데 asset이 없으면 정상적이지 않음
        return m_repositories.AcquireFromAsset<MeshRepository>(desc, asset);
    }

    return m_repositories.Acquire<MeshRepository>(desc);
}

bool RenderResources::ReleaseMesh(MeshHandle mh)
{
    return m_repositories.Release<MeshRepository>(mh);
}

DebugMeshHandle RenderResources::LoadDebugMesh(const DebugMeshDesc& desc, std::shared_ptr<MeshAsset> asset)
{
    if (desc.GetResourceID().GetType() != Core::ResourceIDType::Path)
    {
        Core::Assert(asset); // runtime/builtin 인데 asset이 없으면 정상적이지 않음
        return m_repositories.AcquireFromAsset<DebugMeshRepository>(desc, asset);
    }

    return m_repositories.Acquire<DebugMeshRepository>(desc);
}

MaterialHandle RenderResources::LoadMaterial(const MaterialDesc& desc)
{
    return m_repositories.Acquire<MaterialRepository>(desc);
}

bool RenderResources::ReleaseMaterial(MaterialHandle mh)
{
    return m_repositories.Release<MaterialRepository>(mh);
}

DebugMaterialHandle RenderResources::LoadDebugMaterial(const DebugMaterialDesc& desc)
{
    auto asset = std::make_shared<DebugMaterialAsset>();
    asset->type = desc.GetType();

    return m_repositories.AcquireFromAsset<DebugMaterialRepository>(desc, std::move(asset));
}

bool RenderResources::ReleaseDebugMaterial(DebugMaterialHandle dmh)
{
    return m_repositories.Release<DebugMaterialRepository>(dmh);
}

BrushHandle RenderResources::LoadBrush(const BrushDesc& desc)
{
    return m_repositories.Acquire<BrushRepository>(desc);
}

bool RenderResources::ReleaseBrush(BrushHandle bh)
{
    return m_repositories.Release<BrushRepository>(bh);
}

EnvironmentHandle RenderResources::LoadEnvironment(const EnvironmentDesc& desc)
{
    return m_repositories.Acquire<EnvironmentRepository>(desc);
}

bool RenderResources::ReleaseEnvironment(EnvironmentHandle eh)
{
    return m_repositories.Release<EnvironmentRepository>(eh);
}

void RenderResources::Update()
{
    m_repositories.UpdateAll();
}

void RenderResources::ReleaseAll()
{
    m_repositories.ReleaseAll();
}