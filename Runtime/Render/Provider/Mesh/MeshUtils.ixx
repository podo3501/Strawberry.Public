export module Runtime.Render.Provider:MeshUtils;

import std;
import Runtime.Render.Core;
import Runtime.Render.Command;

export struct UploadRegion
{
    const void* data{ nullptr };    // 보낼 정보 Pointer
    std::uint64_t size{ 0 };        // 버퍼 크기
    std::uint64_t srcOffset{ 0 };   // Upload Resource 내부 Offset
    Resource dstBuffer;            // 대상 버퍼
};

export inline void UploadBufferRegion(CommandList& cmd, Resource& uploadRes, const UploadRegion& region)
{
    std::uint8_t* mapped = nullptr;
    uploadRes->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
    std::memcpy(mapped + region.srcOffset, region.data, region.size);
    uploadRes->Unmap(0, nullptr);

    cmd->CopyBufferRegion(
        region.dstBuffer.Get(),
        0,
        uploadRes.Get(),
        region.srcOffset,
        region.size
    );
}