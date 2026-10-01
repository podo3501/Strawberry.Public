export module Client.Render.View:SceneData;

import std;
import :ID;
import :SceneContext;
import :SceneDrawList;

export struct SceneViewData
{
    SceneViewContext context{ InvalidViewID };
    SceneViewDrawList draws;
};