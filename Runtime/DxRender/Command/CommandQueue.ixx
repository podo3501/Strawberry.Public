module;

#include <d3d12.h>
#include <wrl/client.h>

export module DxRender.Command:Queue;

import std;
import :Type;
import :List;
import DxRender.Core;
import DxRender.Constants;

using Microsoft::WRL::ComPtr;

export class CommandQueue
{
public:
    ~CommandQueue();
    CommandQueue() = default;

    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;
    CommandQueue(CommandQueue&&) noexcept = default;
    CommandQueue& operator=(CommandQueue&&) noexcept = default;

    bool Initialize(
        Device& device,
        ID3D12DescriptorHeap* bindlessHeap,
        CommandType type,
        std::uint32_t cmdPoolSize);

    CommandList* Begin(std::uint32_t slot);
    CommandList* Begin();
    FenceID End();

    // Render 전용: Begin()~End() 사이에서만 유효
    std::vector<CommandList*> BeginParallel(std::size_t count);
    CommandList* EndParallel(std::span<CommandList*> cmdLists);

    void AbortFrame();
    FenceID Signal();
    void WaitIdle();

    ID3D12CommandQueue* GetQueue() const { return m_queue.Get(); }
    FenceID GetCurrentFence() const noexcept { return m_fenceID; }
    FenceID GetCompletedFence() const noexcept { return m_fence->GetCompletedValue(); }

private:
    bool CreateQueue(Device& device, CommandType type);
    bool CreateFence(Device& device);
    void PrepareCommandList(CommandList& cmd);

    void WaitFence(FenceID fenceID);
    void InjectArtificialGpuDelay(std::uint32_t delayMs);
    CommandList* GetAvailableCommandList();

private:
    ID3D12DescriptorHeap* m_bindlessHeap{ nullptr };
    ComPtr<ID3D12CommandQueue> m_queue;
    ComPtr<ID3D12Fence> m_fence;

    FenceID m_fenceID{ InvalidFenceID };
    HANDLE m_event{ nullptr };

    std::vector<CommandList> m_pool;
    std::size_t m_next{ 0 };
    FenceID m_lastSubmittedFence{ InvalidFenceID };

    CommandList* m_currentCmdEntry{ nullptr };
    std::vector<CommandList*> m_pendingSubmission;

    FenceID m_frameFences[FrameBufferCount]{};
    UINT m_pendingFrameSlot{ UINT_MAX };

    // 테스트용 임시 변수
    ComPtr<ID3D12Fence> m_delayFence;
    UINT64 m_delayFenceValue{ 0 };
    std::uint32_t m_artificialDelayMs{ 0 };
};