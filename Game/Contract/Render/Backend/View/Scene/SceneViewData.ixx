export module Contract.Render.Backend.View:SceneData;

import std;
import Contract.Render.Shared.View;
import :SceneDrawList;

export struct SceneViewData
{
    SceneViewContext context{ InvalidViewID };
    SceneViewDrawList draws;
};