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
    ~CommandQueue(); // GPU가 목록과 할당자를 다 쓸 때까지 기다린 뒤 해제한다
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
    CommandList* EndParallel(const std::vector<CommandList*>& cmdLists);

    void AbortFrame();
    FenceID Signal();
    void WaitIdle();
    void WaitOnGpu(const CommandQueue& producer, FenceID fenceID); // producer 큐가 fenceID까지 끝낸 뒤에 이 큐의 다음 제출이 돌게 한다

    ID3D12CommandQueue* GetQueue() const { return m_queue.Get(); }
    FenceID GetCurrentFence() const noexcept { return m_fenceID; }
    FenceID GetCompletedFence() const noexcept { return m_fence->GetCompletedValue(); }

private:
    bool CreateQueue(Device& device, CommandType type);
    bool CreateFence(Device& device, CommandType type);
    void PrepareCommandList(CommandList& cmd);

    void WaitFence(FenceID fenceID);
    CommandList* GetAvailableCommandList();

    // 이벤트 핸들을 소유한다. 이동하면 원본은 비어서 핸들이 두 번 닫히지 않는다.
    struct EventHandle
    {
        HANDLE handle{ nullptr };

        ~EventHandle() { Close(); }
        EventHandle() = default;
        EventHandle(const EventHandle&) = delete;
        EventHandle& operator=(const EventHandle&) = delete;
        EventHandle(EventHandle&& other) noexcept : handle{ std::exchange(other.handle, nullptr) } {}
        EventHandle& operator=(EventHandle&& other) noexcept
        {
            if (this != &other)
            {
                Close();
                handle = std::exchange(other.handle, nullptr);
            }
            return *this;
        }

    private:
        void Close() noexcept
        {
            if (handle)
            {
                CloseHandle(handle);
                handle = nullptr;
            }
        }
    };

private:
    ID3D12DescriptorHeap* m_bindlessHeap{ nullptr };
    ComPtr<ID3D12CommandQueue> m_queue;
    ComPtr<ID3D12Fence> m_fence;

    FenceID m_fenceID{ InvalidFenceID };
    EventHandle m_event;

    std::vector<CommandList> m_pool;
    std::size_t m_next{ 0 };
    FenceID m_lastSubmittedFence{ InvalidFenceID };

    CommandList* m_currentCmdEntry{ nullptr };
    std::vector<CommandList*> m_pendingSubmission;

    FenceID m_frameFences[FrameBufferCount]{};
    UINT m_pendingFrameSlot{ UINT_MAX };
};