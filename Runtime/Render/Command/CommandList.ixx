module;

#include <d3d12.h>
#include <wrl/client.h>

export module Runtime.Render.Command:List;

import std;
import :Type;
import :Conversions;
import Core.Assert;
import Runtime.Render.Core;

export class CommandList
{
public:
    ~CommandList() = default;
    CommandList() = default;

    CommandList(const CommandList&) = delete;
    CommandList& operator=(const CommandList&) = delete;
    CommandList(CommandList&&) noexcept = default;
    CommandList& operator=(CommandList&&) noexcept = default;

    bool Initialize(Device& device, CommandType type)
    {
        if (!device->CreateCommandAllocator(ToD3D12(type), IID_PPV_ARGS(&m_allocator))) return false;
        if (!device->CreateCommandList(0, ToD3D12(type), m_allocator.Get(), nullptr, IID_PPV_ARGS(&m_command))) return false;
        m_command->Close(); // 초기 상태는 닫아둠

        m_type = type;
        return true;
    }

    void SetBindlessHeap(ID3D12DescriptorHeap* heap)
    {
        Core::Assert(heap);

        ID3D12DescriptorHeap* heaps[] = { heap };
        m_command->SetDescriptorHeaps(1, heaps);
    }

    void SetGraphicsRootSignature(ID3D12RootSignature* rootSignature)
    {
        Core::Assert(rootSignature);

        if (m_currentRootSignature == rootSignature)
            return;

        m_command->SetGraphicsRootSignature(rootSignature);
        m_currentRootSignature = rootSignature;
    }

    void SetPipelineState(
        ID3D12PipelineState* pso,
        std::optional<PrimitiveTopologyType> topology = std::nullopt)
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

    void Reset()
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

    void Close()
    {
        Core::Assert(m_state == CmdState::Recording);
        DxCheck(m_command->Close());

        m_state = CmdState::PendingSubmit;
    }

    bool IsAvailable() const
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

    void MarkSubmitted(ID3D12Fence* fence, FenceID fenceID)
    {
        Core::Assert(m_state == CmdState::PendingSubmit); // Close 없이 호출되는 실수 경로 차단
        Core::Assert(fence);
        Core::Assert(fenceID != 0);

        m_fence = fence;
        m_lastFenceID = fenceID;
        m_state = CmdState::InFlight;
    }

    void Discard()
    {
        Core::Assert(m_state == CmdState::Recording || m_state == CmdState::PendingSubmit);

        m_state = CmdState::Ready;
    }

    ID3D12GraphicsCommandList* operator->() const { return m_command.Get(); }
    ID3D12GraphicsCommandList* Get() { return m_command.Get(); }

private:
    enum class CmdState
    {
        Ready,          // 재사용 가능한 초기/유휴 상태
        Recording,      // CPU가 기록 중 (Reset ~ Close 사이)
        PendingSubmit,  // Close됨, 아직 큐에 제출(MarkSubmitted) 안 됨
        InFlight,       // 제출됨, fence로 GPU 완료를 기다리는 중
    };

    CommandType m_type{ CommandType::None };
    mutable CmdState m_state{ CmdState::Ready };

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_allocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command;

    ID3D12RootSignature* m_currentRootSignature{ nullptr };
    ID3D12PipelineState* m_currentPSO{ nullptr };
    ID3D12Fence* m_fence{ nullptr }; // GPU가 처리 중인 Fence 객체

    FenceID m_lastFenceID{ InvalidFenceID }; // 0값은 초기/무효값
};