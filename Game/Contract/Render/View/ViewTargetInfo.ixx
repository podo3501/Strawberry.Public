export module Contract.Render.View:TargetInfo;

import std;
import :ID;
import :CameraData;
import Core.Math;

export struct ViewTargetInfo
{
    ViewID id{ InvalidViewID };
    CameraData camera;
    std::optional<Core::Rect> viewport{ std::nullopt };
};