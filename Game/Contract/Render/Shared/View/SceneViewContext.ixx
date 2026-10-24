export module Contract.Render.Shared.View:SceneViewContext;

import std;
import :ViewID;
import :ViewTargetInfo;
import Contract.Render.Shared.Types;

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