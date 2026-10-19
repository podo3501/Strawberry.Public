module;

#include <d3d12.h>
#include <wrl/client.h>

module DxRender.Command;

import :List;
import :Conversions;
import DxRender.DxCheck;
import DxRender.Core;
import Core.Assert;

bool CommandList::Initialize(Device& device, CommandType type)
{
    if (FAILED(device->CreateCommandAllocator(ToD3D12(type), IID_PPV_ARGS(&m_allocator)))) return false;
    if (FAILED(device->CreateCommandList(0, ToD3D12(type), m_allocator.Get(), nullptr, IID_PPV_ARGS(&m_command)))) return false;
    if (FAILED(m_command->Close())) return false; // 초기 상태는 닫아둠

    m_type = type;
    return true;
}

void CommandList::SetBindlessHeap(ID3D12DescriptorHeap* heap)
{
    Core::Assert(heap);

    ID3D12DescriptorHeap* heaps[] = { heap };
    m_command->SetDescriptorHeaps(1, heaps);
}

void CommandList::SetGraphicsRootSignature(ID3D12RootSignature* rootSignature)
{
    Core::Assert(rootSignature);

    if (m_currentRootSignature == rootSignature)
        return;

    m_command->SetGraphicsRootSignature(rootSignature);
    m_currentRootSignature = rootSignature;
}

void CommandList::SetPipelineState(
    ID3D12PipelineState* pso,
    std::optional<PrimitiveTopologyType> topology)
{
    Core::Assert(pso);

    if (m_currentPSO != pso)
    {
        m_command->SetPipelineState(pso);
        m_currentPSO = pso;
    }

    if (topology)
        m_command->IASetPrimitiveTopology(ToD3D12_Draw(*topology));
}

void CommandList::Reset()
{
    Core::Assert(m_state == CmdState::Ready);

    if (m_lastFenceID != 0)
    {
        Core::Assert(m_fence);
        Core::Assert(m_fence->GetCompletedValue() >= m_lastFenceID); // 이전 GPU 작업 완료 확인
    }

    DxCheck(m_allocator->Reset());
    DxCheck(m_command->Reset(m_allocator.Get(), nullptr));
    m_currentRootSignature = nullptr;
    m_currentPSO = nullptr;

    m_state = CmdState::Recording;
}

void CommandList::Close()
{
    Core::Assert(m_state == CmdState::Recording);
    DxCheck(m_command->Close());

    m_state = CmdState::PendingSubmit;
}

bool CommandList::IsAvailable() const
{
    switch (m_state)
    {
    case CmdState::Ready:
        return true;

    case CmdState::InFlight:
        Core::Assert(m_fence);
        if (m_fence->GetCompletedValue() >= m_lastFenceID) // GPU 작업 완결 여부 확인
        {
            m_state = CmdState::Ready; // fence 완료 확인 시 상태를 확정적으로 되돌림
            return true;
        }
        return false;

    case CmdState::Recording:
    case CmdState::PendingSubmit:
    default:
        return false;
    }
}

void CommandList::MarkSubmitted(ID3D12Fence* fence, FenceID fenceID)
{
    Core::Assert(m_state == CmdState::PendingSubmit); // Close 없이 호출되는 실수 경로 차단
    Core::Assert(fence);
    Core::Assert(fenceID != 0);

    m_fence = fence;
    m_lastFenceID = fenceID;
    m_state = CmdState::InFlight;
}

void CommandList::Discard()
{
    Core::Assert(m_state == CmdState::Recording || m_state == CmdState::PendingSubmit);

    m_state = CmdState::Ready;
}