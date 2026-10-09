module;

#include <dxgiformat.h>

export module DxRender.Definition:RenderFormat;

namespace RenderFormat
{
    export constexpr DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    export constexpr DXGI_FORMAT BackBufferSRGBView = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    export constexpr DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT;
    export constexpr DXGI_FORMAT ShadowMapFormat = DXGI_FORMAT_D32_FLOAT;
}