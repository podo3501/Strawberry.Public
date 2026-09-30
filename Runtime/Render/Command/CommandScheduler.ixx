module;

#include <d3d12.h>

export module Runtime.Render.Command:Scheduler;

import std;
import :Type;
import :List;
import :Queue;
import Core.Assert;
import Runtime.Render.Core;
import Client.Render.Definition;

export class CommandScheduler
{
public:
    ~CommandScheduler()
    {
        WaitIdle();
    }

    CommandScheduler() = default;

    CommandScheduler(const CommandScheduler&) = delete;
    CommandScheduler& operator=(const CommandScheduler&) = delete;
    CommandScheduler(CommandScheduler&&) noexcept = default;
    CommandScheduler& operator=(CommandScheduler&&) noexcept = default;

    bool Initialize(
        Device& device,
        ID3D12DescriptorHeap* bindlessHeap,
        const CommandPoolConfig& config)
    {
        if (!m_directQueue.Initialize(device, bindlessHeap, CommandType::Direct, config.direct)) return false;
        if (!m_copyQueue.Initialize(device, nullptr, CommandType::Copy, config.copy)) return false;
        if (!m_computeQueue.Initialize(device, bindlessHeap, CommandType::Compute, config.compute)) return false;

        return true;
    }

    CommandList* Begin(std::uint32_t slot) //Render 전용
    {
        Core::Assert(!m_currentQueue);

        m_currentQueue = GetQueue(CommandType::Direct);
        auto cmd = m_currentQueue->Begin(slot);
        if (!cmd) m_currentQueue = nullptr;

        return cmd;
    }

    FenceID End() // End -> Close + Signal, PendingRelease 등록
    {
        Core::Assert(m_currentQueue);

        auto fenceID = m_currentQueue->End();
        m_currentQueue = nullptr;
        return fenceID;
    }

    std::vector<CommandList*> BeginParallel(std::size_t count) // Render 전용: Begin()~End() 사이에서만 유효
    {
        Core::Assert(m_currentQueue); // Begin()으로 연 프레임 도중이어야 함
        return m_currentQueue->BeginParallel(count);
    }

    CommandList* EndParallel(std::span<CommandList*> cmds) // Render 전용: Begin()~End() 사이에서만 유효
    {
        Core::Assert(m_currentQueue);
        return m_currentQueue->EndParallel(cmds);
    }

    void AbortFrame()
    {
        Core::Assert(m_currentQueue); // Begin()으로 연 프레임 도중이어야 함

        m_currentQueue->AbortFrame();
        m_currentQueue = nullptr; // End()와 동일하게 "프레임 종료" 상태로 되돌림
    }

    FenceID SignalQueue(CommandType type)
    {
        return GetQueue(type)->Signal();
    }

    void WaitIdle(CommandType type)
    {
        GetQueue(type)->WaitIdle();
    }

    void WaitIdle()
    {
        m_directQueue.WaitIdle();
        m_copyQueue.WaitIdle();
        m_computeQueue.WaitIdle();
    }

    bool IsFenceComplete(CommandType type, FenceID fenceID)
    {
        if (fenceID == 0)
            return true;

        return GetQueue(type)->GetCompletedFence() >= fenceID;
    }

    CommandQueue* GetQueue(CommandType type) noexcept
    {
        switch (type)
        {
        case CommandType::Direct:  return &m_directQueue;
        case CommandType::Copy:    return &m_copyQueue;
        case CommandType::Compute: return &m_computeQueue;
        }

        return nullptr;
    }

private:
    CommandQueue m_directQueue;
    CommandQueue m_copyQueue;
    CommandQueue m_computeQueue;

    CommandQueue* m_currentQueue{ nullptr };
};