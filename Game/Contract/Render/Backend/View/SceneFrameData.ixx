export module Contract.Render.Backend.View:SceneFrameData;

import std;
import :DrawShadowCasterItem;
import :SceneData;
import :OverlayData;
import Contract.Render.Shared.View;

export struct SceneFrameData
{
    DirectionalLightData light;
    std::vector<DrawShadowCasterItem> shadowCasters;
    std::vector<SceneViewData> sceneViews;
    std::vector<OverlayViewData> overlayViews;

    void Clear()
    {
        light = {};
        shadowCasters.clear();
        sceneViews.clear();
        overlayViews.clear();
    }
};