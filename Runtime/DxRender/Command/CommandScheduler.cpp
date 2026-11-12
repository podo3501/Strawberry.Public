module DxRender.Command;

import std;
import :Scheduler;
import Core.Assert;
import Core.Logger;

CommandScheduler::~CommandScheduler()
{
    WaitIdle();
}

bool CommandScheduler::Initialize(
    Device& device,
    ID3D12DescriptorHeap* bindlessHeap,
    const CommandPoolConfig& config)
{
    if (!m_directQueue.Initialize(device, bindlessHeap, CommandType::Direct, config.direct)) return false;
    if (!m_copyQueue.Initialize(device, nullptr, CommandType::Copy, config.copy)) return false;
    if (!m_computeQueue.Initialize(device, bindlessHeap, CommandType::Compute, config.compute)) return false;

    return true;
}

CommandList* CommandScheduler::Begin(std::uint32_t slot) //Render 전용
{
    Core::Assert(!m_currentQueue);

    m_currentQueue = GetQueue(CommandType::Direct);
    auto cmd = m_currentQueue->Begin(slot);
    if (!cmd) m_currentQueue = nullptr;

    return cmd;
}

FenceID CommandScheduler::End() // End -> Close + Signal, PendingRelease 등록
{
    Core::Assert(m_currentQueue);

    auto fenceID = m_currentQueue->End();
    m_currentQueue = nullptr;
    return fenceID;
}

std::vector<CommandList*> CommandScheduler::BeginParallel(std::size_t count) // Render 전용: Begin()~End() 사이에서만 유효
{
    Core::Assert(m_currentQueue); // Begin()으로 연 프레임 도중이어야 함
    return m_currentQueue->BeginParallel(count);
}

CommandList* CommandScheduler::EndParallel(const std::vector<CommandList*>& cmds) // Render 전용: Begin()~End() 사이에서만 유효
{
    Core::Assert(m_currentQueue);
    return m_currentQueue->EndParallel(cmds);
}

void CommandScheduler::AbortFrame()
{
    Core::Assert(m_currentQueue); // Begin()으로 연 프레임 도중이어야 함

    m_currentQueue->AbortFrame();
    m_currentQueue = nullptr; // End()와 동일하게 "프레임 종료" 상태로 되돌림
}

FenceID CommandScheduler::SignalQueue(CommandType type)
{
    CommandQueue* queue = GetQueue(type);
    if (!queue)
    {
        Core::Logger::Error("CommandScheduler::SignalQueue: 명령 종류에 맞는 큐가 없다 (type={})", static_cast<int>(type));
        return InvalidFenceID;
    }

    return queue->Signal();
}

void CommandScheduler::WaitIdle(CommandType type)
{
    CommandQueue* queue = GetQueue(type);
    if (!queue)
    {
        Core::Logger::Error("CommandScheduler::WaitIdle: 명령 종류에 맞는 큐가 없다 (type={})", static_cast<int>(type));
        return;
    }

    queue->WaitIdle();
}

void CommandScheduler::WaitIdle()
{
    m_directQueue.WaitIdle();
    m_copyQueue.WaitIdle();
    m_computeQueue.WaitIdle();
}

bool CommandScheduler::IsFenceComplete(CommandType type, FenceID fenceID)
{
    if (fenceID == InvalidFenceID)
        return true;

    CommandQueue* queue = GetQueue(type);
    if (!queue)
    {
        // 기다릴 큐가 없으니 끝난 것으로 본다 (false면 작업 스케줄러가 영영 기다린다)
        Core::Logger::Error("CommandScheduler::IsFenceComplete: 명령 종류에 맞는 큐가 없다 (type={})", static_cast<int>(type));
        return true;
    }

    return queue->GetCompletedFence() >= fenceID;
}

CommandQueue* CommandScheduler::GetQueue(CommandType type) noexcept
{
    switch (type)
    {
    case CommandType::Direct:  return &m_directQueue;
    case CommandType::Copy:    return &m_copyQueue;
    case CommandType::Compute: return &m_computeQueue;
    }

    return nullptr;
}