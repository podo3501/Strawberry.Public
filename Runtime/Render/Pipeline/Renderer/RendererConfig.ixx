export module Pipeline.Renderer:Config;

import std;

export struct ShadowRendererConfig
{
    std::uint32_t maxObjectCount = 16384;
    std::uint32_t frameCBCount = 1; // Shadow는 프레임당 뷰에 상관없이 한번만 돌기 때문에 1로 설정
};

export struct SurfaceRendererConfig
{
    std::uint32_t maxObjectCount = 16384;
};

export struct DebugSurfaceRendererConfig
{
    std::uint32_t maxObjectCount = 16384;
};

export struct UIRendererConfig
{
    std::uint32_t maxUI = 16384;
};

export struct SkyboxRendererConfig
{
};

export struct RendererConfig
{
    ShadowRendererConfig shadow;
    SurfaceRendererConfig surface;
    DebugSurfaceRendererConfig debug;
    UIRendererConfig ui;
    SkyboxRendererConfig skybox;
};