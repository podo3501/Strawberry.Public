module;

#include <wrl.h>
#include "d3dx12.h"

export module DxRender.Allocator:FrameConstant;

import std;
import Core.Bit;
import DxRender.Core;
import DxRender.Constants;

export class FrameConstantAllocator
{
public:
    // 0이 아닌 유효한 GPU 주소는 이 값과 다르다. 할당에 실패하면 이 값을 돌려준다.
    static constexpr D3D12_GPU_VIRTUAL_ADDRESS InvalidAddress = 0;

    ~FrameConstantAllocator() = default;
    FrameConstantAllocator() = default;

    FrameConstantAllocator(const FrameConstantAllocator&) = delete;
    FrameConstantAllocator& operator=(const FrameConstantAllocator&) = delete;
    FrameConstantAllocator(FrameConstantAllocator&&) noexcept = default;
    FrameConstantAllocator& operator=(FrameConstantAllocator&&) noexcept = default;

    // 슬롯마다 T를 count개 담는다. count가 0이거나 버퍼 크기가 32비트를 넘으면 Error와 false다.
    template<typename T>
    bool Initialize(Device& device, UINT count)
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");

        return InitializeBuffer(
            device,
            static_cast<UINT>(Core::AlignUp(sizeof(T), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)),
            count);
    }

    // 슬롯의 개수를 넘으면 InvalidAddress다 (슬롯을 한 번 쓰는 동안 Error는 한 건만 남는다). 부르는 쪽이 그 항목을 건너뛴다.
    template<typename T>
    [[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS AllocateConstant(const T& data)
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable.");

        return AllocateBytes(&data, sizeof(T));
    }

    void Reset(std::uint32_t slot);

private:
    bool InitializeBuffer(Device& device, UINT stride, UINT count);
    bool CreateBuffer(Device& device, UINT bufferSize);
    D3D12_GPU_VIRTUAL_ADDRESS AllocateBytes(const void* data, std::size_t size);

    Resource m_resource;
    std::uint8_t* m_mapped{ nullptr };
    std::atomic<std::uint64_t> m_offset{ 0 }; // 한도를 넘어 계속 불려도 되감기지 않게 64비트로 둔다
    std::atomic<bool> m_overflowLogged{ false };
    UINT m_stride{ 0 };
    UINT m_perSlotSize{ 0 };
    UINT m_slotBaseOffset{ 0 };
};