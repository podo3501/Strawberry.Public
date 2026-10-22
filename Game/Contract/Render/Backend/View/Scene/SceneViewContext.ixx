export module Contract.Render.Backend.View:SceneContext;

import std;
import :ID;
import :TargetInfo;
import Contract.Render.Backend.Types;

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