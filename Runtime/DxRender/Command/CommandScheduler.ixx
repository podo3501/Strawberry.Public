module;

#include <d3d12.h>

export module DxRender.Command:Scheduler;

import std;
import :Type;
import :List;
import :Queue;
import DxRender.Core;
import Contract.Render.Backend.Types;

export class CommandScheduler
{
public:
    ~CommandScheduler();
    CommandScheduler() = default;

    // 다른 곳이 이 객체의 참조를 들고 있고 m_currentQueue가 자기 멤버를 가리키므로 복사도 이동도 하지 않는다.
    CommandScheduler(const CommandScheduler&) = delete;
    CommandScheduler& operator=(const CommandScheduler&) = delete;
    CommandScheduler(CommandScheduler&&) = delete;
    CommandScheduler& operator=(CommandScheduler&&) = delete;

    bool Initialize(
        Device& device,
        ID3D12DescriptorHeap* bindlessHeap,
        const CommandPoolConfig& config);

    CommandList* Begin(std::uint32_t slot); //Render 전용
    FenceID End(); // End -> Close + Signal, PendingRelease 등록
    
    std::vector<CommandList*> BeginParallel(std::size_t count); // Render 전용: Begin()~End() 사이에서만 유효
    CommandList* EndParallel(const std::vector<CommandList*>& cmds); // Render 전용: Begin()~End() 사이에서만 유효

    void AbortFrame();
    // 큐가 없는 종류(None 등)는 Error를 남긴다: SignalQueue는 InvalidFenceID, WaitIdle은 기다리지 않음, IsFenceComplete는 true
    FenceID SignalQueue(CommandType type);
    void WaitIdle(CommandType type);
    void WaitIdle();

    bool IsFenceComplete(CommandType type, FenceID fenceID);
    CommandQueue* GetQueue(CommandType type) noexcept; // 큐가 없는 종류는 nullptr

private:
    CommandQueue m_directQueue;
    CommandQueue m_copyQueue;
    CommandQueue m_computeQueue;

    CommandQueue* m_currentQueue{ nullptr };
};