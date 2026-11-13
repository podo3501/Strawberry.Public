module;

#include <d3d12.h>
#include <wrl.h>

export module DxRender.Diagnostics:FrameProfiler;

import std;
import DxRender.Core;
import DxRender.Command;
import DxRender.Constants;
import DxRender.Factory;

// 프레임 슬롯(FrameBufferCount개)마다 GPU 시작/끝 시간을 재고, 제출한 펜스로 GPU에서 끝난 슬롯만 읽는다.
export class FrameProfiler
{
public:
    FrameProfiler() = default;
    ~FrameProfiler() = default;

    // 시작할 때 한 번만 부른다. cmdScheduler는 이 객체보다 오래 살아야 한다.
    bool Initialize(Device& device, CommandScheduler& cmdScheduler, ResourceFactory& resFactory);
    void BeginFrame(CommandList& cmd, std::uint64_t frameIndex);
    void EndFrame(CommandList& cmd);
    void MarkSubmitted(FenceID fence); // 이번 프레임을 CommandScheduler::End()로 제출한 직후 그 펜스를 알린다.
    void Update(std::uint64_t frameIndex);

    [[nodiscard]] float GetCpuFrameTimeMs() const noexcept { return m_cpuFrameTimeMs; }
    [[nodiscard]] float GetGpuFrameTimeMs() const noexcept { return m_gpuFrameTimeMs; }

private:
    CommandScheduler* m_cmdScheduler{ nullptr };
    Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_queryHeap;
    Resource m_readbackResource;

    std::uint64_t m_timestampFreq{ 0 };

    UINT m_currentSlot{ 0 };
    std::array<FenceID, FrameBufferCount> m_slotFences{}; // 슬롯에 마지막으로 제출한 프레임의 펜스 (InvalidFenceID는 제출한 적 없음)
    float m_gpuFrameTimeMs{ 0.0f };

    std::chrono::high_resolution_clock::time_point m_cpuStart;
    float m_cpuFrameTimeMs{ 0.0f };
};
