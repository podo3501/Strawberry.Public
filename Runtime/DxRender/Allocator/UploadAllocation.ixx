module;

#include <d3d12.h>

export module DxRender.Allocator:UploadAllocation;

import std;
import DxRender.Core;

// 할당된 영역의 CPU/GPU 주소를 반환하는 구조체
export struct UploadAllocation
{
    Resource* resource{ nullptr };
    std::uint8_t* cpuAddress{ nullptr };
    D3D12_GPU_VIRTUAL_ADDRESS gpuAddress{ 0 };
    std::size_t offset{ 0 };
    UINT sizeInBytes{ 0 };

    explicit operator bool() const noexcept
    {
        return cpuAddress != nullptr;
    }
};
