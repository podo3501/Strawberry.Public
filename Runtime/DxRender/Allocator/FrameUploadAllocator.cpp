module;

#include "d3dx12.h"

module DxRender.Allocator;

import :FrameUpload;
import DxRender.Constants;
import Core.Assert;
import Core.Bit;
import Core.Logger;

bool FrameUploadAllocator::Initialize(
        Device& device,
        UINT bufferSizeInBytes,
        UINT elementStride)
    {
        Core::Assert(bufferSizeInBytes > 0);
        Core::Assert(elementStride > 0);

        // 오프셋이 UINT라서 슬롯 전체 크기가 UINT에 담겨야 한다. 64비트로 계산해 넘치면 실패시킨다
        const std::uint64_t perSlotSize = Core::AlignUpGeneric(
            static_cast<std::uint64_t>(bufferSizeInBytes), static_cast<std::uint64_t>(elementStride));
        if (perSlotSize * FrameBufferCount > std::numeric_limits<UINT>::max())
        {
            Core::Logger::Error("FrameUploadAllocator::Initialize: 슬롯 {}개의 버퍼 크기가 UINT를 넘는다 (슬롯당 {}바이트)", FrameBufferCount, perSlotSize);
            return false;
        }

        m_elementStride = elementStride;
        m_perSlotSize = static_cast<UINT>(perSlotSize);

        UINT slotCapacityInElements = m_perSlotSize / elementStride;
        for (auto& region : m_region)
        {
            if (!region.Initialize(slotCapacityInElements))
                return false;
        }

        m_resource = device.CreateResource(
            CD3DX12_RESOURCE_DESC::Buffer(m_perSlotSize * FrameBufferCount), // 슬롯 수만큼 버퍼를 늘려서, 슬롯마다 겹치지 않는 영역을 갖게 한다.
            D3D12_HEAP_TYPE_UPLOAD,
            D3D12_RESOURCE_STATE_GENERIC_READ);

        if (!m_resource)
            return false;

        auto hr = m_resource->Map(0, nullptr, reinterpret_cast<void**>(&m_mapped));
        if (FAILED(hr))
        {
            Core::Logger::Error("FrameUploadAllocator::Initialize: 버퍼를 매핑하지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
            return false;
        }

        return true;
    }

    UploadAllocation FrameUploadAllocator::Allocate(UINT elementCount) noexcept
    {
        Core::Assert(m_elementStride > 0); // Initialize 등록 없이 Allocate 호출 금지

        UINT localElement = m_region[m_currentSlot].Allocate(elementCount);
        if (localElement == Core::InvalidIndex) // 용량 초과나 0개 요청. 원인은 Core 할당기가 이미 남겼으니 빈 값만 돌려주고 호출하는 쪽이 처리한다
            return {};

        UINT sizeInBytes = elementCount * m_elementStride;
        UINT globalOffset = m_perSlotSize * m_currentSlot + localElement * m_elementStride; // slot offset + element offset

        UploadAllocation alloc;
        alloc.resource = &m_resource;
        alloc.cpuAddress = m_mapped + globalOffset;
        alloc.gpuAddress = m_resource->GetGPUVirtualAddress() + globalOffset;
        alloc.offset = globalOffset;
        alloc.sizeInBytes = sizeInBytes;

        return alloc;
    }

    void FrameUploadAllocator::Reset(std::uint32_t slot)
    {
        Core::Assert(slot < FrameBufferCount);

        m_currentSlot = slot;
        m_region[slot].Reset();
    }