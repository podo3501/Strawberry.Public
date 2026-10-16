module;

#include <d3d12.h>

export module DxRender.Command:Scheduler;

import std;
import :Type;
import :List;
import :Queue;
import DxRender.Core;
import Contract.Render.Types;

export class CommandScheduler
{
public:
    ~CommandScheduler();
    CommandScheduler() = default;

    CommandScheduler(const CommandScheduler&) = delete;
    CommandScheduler& operator=(const CommandScheduler&) = delete;
    CommandScheduler(CommandScheduler&&) noexcept = default;
    CommandScheduler& operator=(CommandScheduler&&) noexcept = default;

    bool Initialize(
        Device& device,
        ID3D12DescriptorHeap* bindlessHeap,
        const CommandPoolConfig& config);

    CommandList* Begin(std::uint32_t slot); //Render 전용
    FenceID End(); // End -> Close + Signal, PendingRelease 등록
    
    std::vector<CommandList*> BeginParallel(std::size_t count); // Render 전용: Begin()~End() 사이에서만 유효
    CommandList* EndParallel(std::span<CommandList*> cmds); // Render 전용: Begin()~End() 사이에서만 유효

    void AbortFrame();
    FenceID SignalQueue(CommandType type);
    void WaitIdle(CommandType type);
    void WaitIdle();

    bool IsFenceComplete(CommandType type, FenceID fenceID);
    CommandQueue* GetQueue(CommandType type) noexcept;

private:
    CommandQueue m_directQueue;
    CommandQueue m_copyQueue;
    CommandQueue m_computeQueue;

    CommandQueue* m_currentQueue{ nullptr };
};