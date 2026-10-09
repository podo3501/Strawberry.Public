export module Contract.Render.View:SceneDrawList;

import std;
import Core.Math;
import Contract.Render.IResource;
import Contract.Render.Definition;

export struct DrawSurfaceItem
{
    std::shared_ptr<IResource> mesh;
    std::shared_ptr<IResource> material;
    std::optional<ShaderID> shaderOverride;
    Core::Matrix world;
};

export struct DrawDebugSurfaceItem
{
    std::shared_ptr<IResource> mesh;
    std::shared_ptr<IResource> material;
    Core::Matrix world;
};

export struct SceneViewDrawList
{
    std::shared_ptr<IResource> environment;
    std::vector<DrawSurfaceItem> surfaces;
    std::vector<DrawDebugSurfaceItem> debugSurfaces;

    bool IsEmpty() const
    {
        return !environment && surfaces.empty() && debugSurfaces.empty();
    }

    void Clear()
    {
        environment.reset();
        surfaces.clear();
        debugSurfaces.clear();
    }
};