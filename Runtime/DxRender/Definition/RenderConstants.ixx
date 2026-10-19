module;

#include <d3d12.h>

export module DxRender.Constants;

import std;

namespace BufferAlignment
{
    export inline constexpr std::size_t VertexBuffer = 16;
    export inline constexpr std::size_t IndexBuffer = 16;
    export inline constexpr std::size_t ConstantBuffer = 256;
}

namespace TextureAlignment
{
    export inline constexpr std::size_t Row = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;       // 256
    export inline constexpr std::size_t Placement = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT; // 512
}

export constexpr std::uint32_t FrameBufferCount = 3;

namespace RenderFormat
{
    export inline constexpr DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    export inline constexpr DXGI_FORMAT BackBufferSRGBView = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    export inline constexpr DXGI_FORMAT DepthFormat = DXGI_FORMAT_D32_FLOAT;
    export inline constexpr DXGI_FORMAT ShadowMapFormat = DXGI_FORMAT_D32_FLOAT;
}