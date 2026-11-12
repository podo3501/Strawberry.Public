module;

#include <d3d12.h>
#include <dxgi.h>

export module DxRender.Helper:D3D12Conversions;

import std;
import DxRender.Definition;
import Contract.Render.Assets;

// 정의되지 않은 값은 D3D12의 "없음"인 UNDEFINED라서 PSO를 만들 때 실패로 드러난다.
export D3D12_PRIMITIVE_TOPOLOGY_TYPE ToD3D12_PSO(PrimitiveTopologyType topology)
{
    switch (topology)
    {
    case PrimitiveTopologyType::Triangle: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case PrimitiveTopologyType::Line:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    }

    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}

// 정의되지 않은 값은 기본인 Solid
export D3D12_FILL_MODE ToD3D12(FillMode mode)
{
    switch (mode)
    {
    case FillMode::Solid:     return D3D12_FILL_MODE_SOLID;
    case FillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
    }

    return D3D12_FILL_MODE_SOLID;
}

// 정의되지 않은 값은 아무것도 걸러내지 않는 None
export D3D12_CULL_MODE ToD3D12(CullMode mode)
{
    switch (mode)
    {
    case CullMode::None:  return D3D12_CULL_MODE_NONE;
    case CullMode::Front: return D3D12_CULL_MODE_FRONT;
    case CullMode::Back:  return D3D12_CULL_MODE_BACK;
    }

    return D3D12_CULL_MODE_NONE;
}

// ------------------------------------------------------------------------
// ToDXGI
// ------------------------------------------------------------------------
export DXGI_FORMAT ToSRGB(DXGI_FORMAT format)
{
    switch (format)
    {
    case DXGI_FORMAT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case DXGI_FORMAT_BC1_UNORM:      return DXGI_FORMAT_BC1_UNORM_SRGB;
    case DXGI_FORMAT_BC2_UNORM:      return DXGI_FORMAT_BC2_UNORM_SRGB;
    case DXGI_FORMAT_BC3_UNORM:      return DXGI_FORMAT_BC3_UNORM_SRGB;
    default:
        return format; // 이미 SRGB거나 변환 불가
    }
}

export DXGI_FORMAT ToDXGIFormat(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::Unknown:     return DXGI_FORMAT_UNKNOWN;
    case PixelFormat::RGB8:        return DXGI_FORMAT_R8G8B8A8_UNORM; // 3채널 미지원
    case PixelFormat::RGBA8:       return DXGI_FORMAT_R8G8B8A8_UNORM;
    case PixelFormat::R11G11B10F:  return DXGI_FORMAT_R11G11B10_FLOAT;
    case PixelFormat::RGBA16F:     return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case PixelFormat::RGB9E5:      return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
    case PixelFormat::BC6H_UF16:   return DXGI_FORMAT_BC6H_UF16;
    }

    return DXGI_FORMAT_R8G8B8A8_UNORM; // 정의되지 않은 값의 기본값
}