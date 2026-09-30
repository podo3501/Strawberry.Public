module;

#include <d3d12.h>

export module Runtime.Render.Constants;

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

export inline constexpr std::uint32_t FrameBufferCount = 3;