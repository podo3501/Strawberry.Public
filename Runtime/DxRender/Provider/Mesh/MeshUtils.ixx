module;

#include <d3d12.h>

export module DxRender.Provider:MeshUtils;

import std;
import DxRender.Core;
import DxRender.Command;
import Core.Logger;

export struct UploadRegion
{
    const void* data{ nullptr };    // 보낼 정보 Pointer
    std::uint64_t size{ 0 };        // 버퍼 크기
    std::uint64_t srcOffset{ 0 };   // Upload Resource 내부 Offset
    Resource dstBuffer;            // 대상 버퍼
};

// 업로드 버퍼에 복사하고 복사 명령을 기록한다. 버퍼를 매핑하지 못하면 Error와 false
export inline bool UploadBufferRegion(CommandList& cmd, Resource& uploadRes, const UploadRegion& region)
{
    std::uint8_t* mapped = nullptr;
    const HRESULT hr = uploadRes->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
    if (FAILED(hr) || !mapped)
    {
        Core::Logger::Error("UploadBufferRegion: 업로드 버퍼를 매핑하지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }
    std::memcpy(mapped + region.srcOffset, region.data, region.size);
    uploadRes->Unmap(0, nullptr);

    cmd->CopyBufferRegion(
        region.dstBuffer.Get(),
        0,
        uploadRes.Get(),
        region.srcOffset,
        region.size
    );
    return true;
}