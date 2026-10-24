export module Contract.Render.Shared.View:ViewTargetInfo;

import std;
import :ViewID;
import :CameraData;
import Core.Math;

export struct ViewTargetInfo
{
    ViewID id{ InvalidViewID };
    CameraData camera;
    std::optional<Core::Rect> viewport{ std::nullopt };
};