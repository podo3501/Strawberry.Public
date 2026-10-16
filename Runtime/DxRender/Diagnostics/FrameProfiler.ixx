module;

#include <d3d12.h>
#include <wrl.h>

export module DxRender.Diagnostics:FrameProfiler;

import std;
import DxRender.Core;
import DxRender.Command;
import DxRender.Factory;

export class FrameProfiler
{
public:
    FrameProfiler() = default;
    ~FrameProfiler() = default;

    bool Initialize(Device& device, CommandScheduler& cmdScheduler,
        ResourceFactory& resFactory, std::uint32_t frameCount = 2);
    void BeginFrame(CommandList& cmd, std::uint64_t frameIndex);
    void EndFrame(CommandList& cmd);
    void Update(std::uint64_t frameIndex);

    [[nodiscard]] float GetCpuFrameTimeMs() const noexcept { return m_cpuFrameTimeMs; }
    [[nodiscard]] float GetGpuFrameTimeMs() const noexcept { return m_gpuFrameTimeMs; }

private:
    Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_queryHeap;
    Resource m_readbackResource;

    std::uint64_t m_timestampFreq{ 0 };

    std::uint32_t m_frameCount{ 0 };
    UINT m_currentSlot{ 0 };
    float m_gpuFrameTimeMs{ 0.0f };

    std::chrono::high_resolution_clock::time_point m_cpuStart;
    float m_cpuFrameTimeMs{ 0.0f };
};