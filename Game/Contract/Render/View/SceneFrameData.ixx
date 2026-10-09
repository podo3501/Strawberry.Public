export module Contract.Render.View:SceneFrameData;

import std;
import :DirectionalLightData;
import :DrawShadowCasterItem;
import :SceneData;
import :OverlayData;

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