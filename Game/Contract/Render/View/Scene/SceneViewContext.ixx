export module Client.Render.View:SceneContext;

import std;
import :ID;
import :TargetInfo;
import Client.Render.Definition;

export struct RenderOverride
{
    std::optional<RasterPreset> rasterPreset;
};

export struct SceneViewContext
{
    explicit SceneViewContext(ViewID id) : target{ id } {}
    ViewTargetInfo target;
    RenderOverride renderOverride;
};