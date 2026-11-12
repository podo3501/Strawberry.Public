module;

#include <d3d12.h>

export module DxRender.Command:Conversions;

import std;
import :Type;
import DxRender.Definition;

// None이나 범위 밖 값은 D3D12에 대응하는 종류가 없어 nullopt
export std::optional<D3D12_COMMAND_LIST_TYPE> ToD3D12(CommandType type)
{
    switch (type)
    {
    case CommandType::Direct:  return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case CommandType::Copy:    return D3D12_COMMAND_LIST_TYPE_COPY;
    case CommandType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    case CommandType::None:    break;
    }

    return std::nullopt;
}

// 정의되지 않은 값은 D3D12의 "없음"인 UNDEFINED
export D3D_PRIMITIVE_TOPOLOGY ToD3D12_Draw(PrimitiveTopologyType topology)
{
    switch (topology)
    {
    case PrimitiveTopologyType::Triangle: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case PrimitiveTopologyType::Line:     return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    }

    return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}
