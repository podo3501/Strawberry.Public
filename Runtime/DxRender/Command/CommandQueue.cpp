module;

#include <d3d12.h>
#include <wrl/client.h>

module DxRender.Command;

import std;
import :Queue;
import :Conversions;
import DxRender.DxCheck;
import DxRender.Core;
import DxRender.Constants;
import Core.Assert;
import Core.Logger;

using Microsoft::WRL::ComPtr;

CommandQueue::~CommandQueue()
{
    if (m_fence) // 이동했거나 초기화하지 않은 큐는 기다릴 것이 없다
        WaitIdle();
}

bool CommandQueue::Initialize(
    Device& device,
    ID3D12DescriptorHeap* bindlessHeap,
    CommandType type,
    std::uint32_t cmdPoolSize)
{
    if (cmdPoolSize == 0)
    {
        Core::Logger::Error("CommandQueue::Initialize: 명령 목록 풀 크기가 0이다 (type={})", static_cast<int>(type));
        return false;
    }

    if (!CreateQueue(device, type)) return false;
    if (!CreateFence(device, type)) return false;

    m_bindlessHeap = bindlessHeap;

    m_pool.resize(cmdPoolSize);
    for (auto& cmd : m_pool)
        if (!cmd.Initialize(device, type)) return false;

    return true;
}

CommandList* CommandQueue::Begin(std::uint32_t slot)
{
    Core::Assert(slot < FrameBufferCount);

    // 0이면 아직 이 슬롯에 제출한 적이 없다는 뜻이므로 대기 없이 통과.
    // (Signal()이 ++m_fenceID로 1부터 시작하므로 0은 "미사용"을 의미)
    if (m_frameFences[slot] != InvalidFenceID)
        WaitFence(m_frameFences[slot]);

    CommandList* cmd = Begin();
    if (cmd)
        m_pendingFrameSlot = slot;

    return cmd;
}

CommandList* CommandQueue::Begin()
{
    Core::Assert(!m_currentCmdEntry);

    auto entry = GetAvailableCommandList();
    if (!entry) return nullptr;

    PrepareCommandList(*entry);
    m_currentCmdEntry = entry;

    return entry;
}

FenceID CommandQueue::End()
{
    Core::Assert(m_currentCmdEntry);

    m_currentCmdEntry->Close();
    m_pendingSubmission.push_back(m_currentCmdEntry);
    m_currentCmdEntry = nullptr;

    std::vector<ID3D12CommandList*> raw;
    raw.reserve(m_pendingSubmission.size());
    for (auto* cmdList : m_pendingSubmission)
        raw.push_back(cmdList->Get());

    m_queue->ExecuteCommandLists(static_cast<UINT>(raw.size()), raw.data());

    FenceID fenceID = Signal();
    m_lastSubmittedFence = fenceID;

    if (m_pendingFrameSlot != UINT_MAX)
    {
        m_frameFences[m_pendingFrameSlot] = fenceID;
        m_pendingFrameSlot = UINT_MAX;
    }

    for (auto* cmdList : m_pendingSubmission)
        cmdList->MarkSubmitted(m_fence.Get(), fenceID);

    m_pendingSubmission.clear();
    return fenceID;
}

// Render 전용: Begin()~End() 사이에서만 유효
std::vector<CommandList*> CommandQueue::BeginParallel(std::size_t count)
{
    Core::Assert(m_currentCmdEntry); // Begin()으로 연 상태에서만 호출 가능

    m_currentCmdEntry->Close();
    m_pendingSubmission.push_back(m_currentCmdEntry);
    m_currentCmdEntry = nullptr;

    std::vector<CommandList*> result;
    result.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        CommandList* entry = GetAvailableCommandList();
        if (!entry) break;

        PrepareCommandList(*entry);
        result.push_back(entry);
    }
    return result;
}

CommandList* CommandQueue::EndParallel(const std::vector<CommandList*>& cmdLists)
{
    Core::Assert(!cmdLists.empty());

    for (auto* entry : cmdLists)
    {
        entry->Close();
        m_pendingSubmission.push_back(entry);
    }

    auto entry = GetAvailableCommandList();
    if (!entry) return nullptr; // pool 고갈 처리

    PrepareCommandList(*entry);
    m_currentCmdEntry = entry;

    return entry;
}

void CommandQueue::AbortFrame()
{
    // 기록 중이던 primary Discard
    if (m_currentCmdEntry)
    {
        m_currentCmdEntry->Discard();
        m_currentCmdEntry = nullptr;
    }

    for (auto* entry : m_pendingSubmission)
        entry->Discard(); // Close는 됐지만 아직 ExecuteCommandLists를 안 탄 것들 전부 폐기

    m_pendingSubmission.clear();
    m_pendingFrameSlot = UINT_MAX;
}

FenceID CommandQueue::Signal()
{
    FenceID id = ++m_fenceID;
    DxCheck(m_queue->Signal(m_fence.Get(), id));
    return id;
}

void CommandQueue::WaitIdle()
{
    if (m_fenceID != InvalidFenceID)
        WaitFence(m_fenceID); // m_fenceID가 마지막으로 신호를 준 값이다
}

bool CommandQueue::CreateQueue(Device& device, CommandType type)
{
    const auto d3dType = ToD3D12(type);
    if (!d3dType)
    {
        Core::Logger::Error("CommandQueue::CreateQueue: 명령 종류가 올바르지 않다 (type={})", static_cast<int>(type));
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = *d3dType;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    if (HRESULT hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue)); FAILED(hr))
    {
        Core::Logger::Error("CommandQueue::CreateQueue: 명령 큐를 만들지 못했다 (type={}, hr={:#010x})", static_cast<int>(type), static_cast<std::uint32_t>(hr));
        return false;
    }

    return true;
}

bool CommandQueue::CreateFence(Device& device, CommandType type)
{
    m_fence = device.CreateFence(0, D3D12_FENCE_FLAG_NONE); // 실패하면 Device가 오류를 남긴다
    if (!m_fence) return false;

    m_event.handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_event.handle)
    {
        Core::Logger::Error("CommandQueue::CreateFence: 대기 이벤트를 만들지 못했다 (type={}, error={})", static_cast<int>(type), GetLastError());
        return false;
    }

    return true;
}

void CommandQueue::PrepareCommandList(CommandList& cmd)
{
    cmd.Reset();

    if (m_bindlessHeap)
        cmd.SetBindlessHeap(m_bindlessHeap);
}

CommandList* CommandQueue::GetAvailableCommandList()
{
    for (std::size_t i = 0; i < m_pool.size(); ++i)
    {
        CommandList* entry = &m_pool[m_next];
        m_next = (m_next + 1) % m_pool.size();

        if (entry->IsAvailable())
            return entry;
    }

    return nullptr; // 사용 가능한 context 없음 여기서 만약 while로 기다리게 되면 cpu, gpu 동기화가 되기 때문에 일부러 nullptr 리턴함. begin에서 nullptr이면 present 안하고 리턴. 의도한 바임.
}

void CommandQueue::WaitFence(FenceID fenceID)
{
    if (m_fence->GetCompletedValue() < fenceID)
    {
        DxCheck(m_fence->SetEventOnCompletion(fenceID, m_event.handle)); // 실패하면 이벤트가 영영 오지 않아 멈추므로 확인한다
        WaitForSingleObject(m_event.handle, INFINITE);
    }
}