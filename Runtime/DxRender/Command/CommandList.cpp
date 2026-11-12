module;

#include <d3d12.h>
#include <wrl/client.h>

module DxRender.Command;

import :List;
import :Conversions;
import DxRender.DxCheck;
import DxRender.Core;
import Core.Assert;
import Core.Logger;

bool CommandList::Initialize(Device& device, CommandType type)
{
    const auto d3dType = ToD3D12(type);
    if (!d3dType)
    {
        Core::Logger::Error("CommandList::Initialize: 명령 종류가 올바르지 않다 (type={})", static_cast<int>(type));
        return false;
    }

    if (HRESULT hr = device->CreateCommandAllocator(*d3dType, IID_PPV_ARGS(&m_allocator)); FAILED(hr))
    {
        Core::Logger::Error("CommandList::Initialize: 명령 할당자를 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }

    if (HRESULT hr = device->CreateCommandList(0, *d3dType, m_allocator.Get(), nullptr, IID_PPV_ARGS(&m_command)); FAILED(hr))
    {
        Core::Logger::Error("CommandList::Initialize: 명령 목록을 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }

    if (HRESULT hr = m_command->Close(); FAILED(hr)) // 초기 상태는 닫아둠
    {
        Core::Logger::Error("CommandList::Initialize: 명령 목록을 닫지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }

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

    if (m_lastFenceID != InvalidFenceID)
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
        return false;
    }

    return false;
}

void CommandList::MarkSubmitted(ID3D12Fence* fence, FenceID fenceID)
{
    Core::Assert(m_state == CmdState::PendingSubmit); // Close 없이 호출되는 실수 경로 차단
    Core::Assert(fence);
    Core::Assert(fenceID != InvalidFenceID);

    m_fence = fence;
    m_lastFenceID = fenceID;
    m_state = CmdState::InFlight;
}

void CommandList::Discard()
{
    Core::Assert(m_state == CmdState::Recording || m_state == CmdState::PendingSubmit);

    if (m_state == CmdState::Recording)
        DxCheck(m_command->Close()); // 열린 채로 두면 다시 Reset할 수 없으므로 닫고 버린다

    m_state = CmdState::Ready;
}