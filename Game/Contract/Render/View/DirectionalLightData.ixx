export module Contract.Render.View:DirectionalLightData;

import Core.Math;

export struct DirectionalLightData
{
    Core::Vector3 direction;
    Core::Vector3 color;
    float intensity{ 0.f };
    Core::Matrix viewProj;
};