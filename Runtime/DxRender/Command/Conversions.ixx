module;

#include <d3d12.h>

export module DxRender.Command:Conversions;

import Core.Assert;
import :Type;
import DxRender.Definition;

export D3D12_COMMAND_LIST_TYPE ToD3D12(CommandType type)
{
    switch (type)
    {
    case CommandType::Direct:  return D3D12_COMMAND_LIST_TYPE_DIRECT;
    case CommandType::Copy:    return D3D12_COMMAND_LIST_TYPE_COPY;
    case CommandType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
    default:
        return D3D12_COMMAND_LIST_TYPE_DIRECT; // 안전 기본값
    }
}

export D3D_PRIMITIVE_TOPOLOGY ToD3D12_Draw(PrimitiveTopologyType topology)
{
    switch (topology)
    {
    case PrimitiveTopologyType::Triangle: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case PrimitiveTopologyType::Line:     return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    default:
        Core::Assert(false);
        return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    }
}