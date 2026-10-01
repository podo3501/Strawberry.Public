export module Client.Render.View:CameraData;

import std;
import Core.Math;

export struct CameraData
{
    Core::Matrix view;
    Core::Matrix proj;
    Core::Vector3 position;
};