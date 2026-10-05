module;

#include <d3d12.h>
#include <wrl.h>

export module Runtime.Render.Diagnostics:FrameProfiler;

import std;
import Runtime.Render.Core;
import Runtime.Render.Command;
import Runtime.Render.Factory;

export class FrameProfiler
{
public:
    FrameProfiler() = default;
    ~FrameProfiler() = default;

    bool Initialize(Device& device, CommandScheduler& cmdScheduler,
        ResourceFactory& resFactory, std::uint32_t frameCount = 2)
    {
        m_frameCount = frameCount;

        auto queue = cmdScheduler.GetQueue(CommandType::Direct)->GetQueue();
        queue->GetTimestampFrequency(&m_timestampFreq);

        m_queryHeap = device.CreateQueryHeap(
            D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
            frameCount * QueriesPerFrame);

        const UINT64 size = sizeof(std::uint64_t) * frameCount * QueriesPerFrame;
        m_readbackResource = resFactory.CreateResource(size, ResInitType::Readback);

        return true;
    }

    void BeginFrame(CommandList& cmd, std::uint64_t frameIndex)
    {
        m_currentSlot = static_cast<UINT>(frameIndex % m_frameCount);

        cmd->EndQuery(
            m_queryHeap.Get(),
            D3D12_QUERY_TYPE_TIMESTAMP,
            m_currentSlot * QueriesPerFrame);

        m_cpuStart = std::chrono::high_resolution_clock::now();

        // 주의: frameIndex는 프레임이 성공적으로 끝났을 때만(EndFrame까지 도달했을 때만)
        // 증가해야 함. 실패/abort 시 같은 frameIndex로 재시도되어야 이 슬롯이 안전하게 재사용됨.
    }

    void EndFrame(CommandList& cmd)
    {
        cmd->EndQuery(
            m_queryHeap.Get(),
            D3D12_QUERY_TYPE_TIMESTAMP,
            m_currentSlot * QueriesPerFrame + 1);

        cmd->ResolveQueryData(
            m_queryHeap.Get(),
            D3D12_QUERY_TYPE_TIMESTAMP,
            m_currentSlot * QueriesPerFrame,
            QueriesPerFrame,
            m_readbackResource.Get(),
            m_currentSlot * sizeof(std::uint64_t) * QueriesPerFrame
        );

        auto now = std::chrono::high_resolution_clock::now();
        m_cpuFrameTimeMs = std::chrono::duration<float, std::milli>(now - m_cpuStart).count();
    }

    void Update(std::uint64_t frameIndex)
    {
        const auto slot = static_cast<UINT>((frameIndex + m_frameCount - 1) % m_frameCount); // 이전 프레임의 GPU 측정 결과를 읽는다.
        std::uint64_t* data = nullptr;

        D3D12_RANGE range =
        {
            slot * sizeof(std::uint64_t) * QueriesPerFrame,
            (slot + 1) * sizeof(std::uint64_t) * QueriesPerFrame
        };

        if (SUCCEEDED(m_readbackResource->Map(
            0,
            &range,
            reinterpret_cast<void**>(&data))))
        {
            const UINT queryIndex = slot * QueriesPerFrame;

            std::uint64_t start = data[queryIndex];
            std::uint64_t end = data[queryIndex + 1];
            m_gpuFrameTimeMs = float(end - start) * 1000.0f / float(m_timestampFreq);

            m_readbackResource->Unmap(0, nullptr);
        }
    }

    [[nodiscard]] float GetCpuFrameTimeMs() const noexcept { return m_cpuFrameTimeMs; }
    [[nodiscard]] float GetGpuFrameTimeMs() const noexcept { return m_gpuFrameTimeMs; }

private:
    static constexpr UINT QueriesPerFrame = 2;

    Microsoft::WRL::ComPtr<ID3D12QueryHeap> m_queryHeap;
    Resource m_readbackResource;

    std::uint64_t m_timestampFreq{ 0 };

    std::uint32_t m_frameCount{ 0 };
    UINT m_currentSlot{ 0 };
    float m_gpuFrameTimeMs{ 0.0f };

    std::chrono::high_resolution_clock::time_point m_cpuStart;
    float m_cpuFrameTimeMs{ 0.0f };
};