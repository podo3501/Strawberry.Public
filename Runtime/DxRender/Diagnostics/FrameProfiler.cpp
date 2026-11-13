module;

#include <d3d12.h>
#include <wrl.h>

module DxRender.Diagnostics;

import std;
import :FrameProfiler;
import Core.Logger;

static constexpr UINT QueriesPerFrame = 2;

bool FrameProfiler::Initialize(Device& device, CommandScheduler& cmdScheduler, ResourceFactory& resFactory)
{
    auto* directQueue = cmdScheduler.GetQueue(CommandType::Direct);
    if (!directQueue || !directQueue->GetQueue())
    {
        Core::Logger::Error("FrameProfiler::Initialize: 직접 큐가 없다 (명령 스케줄러를 먼저 초기화해야 한다)");
        return false;
    }

    // 주파수가 0이면 GPU 시간을 0으로 나누게 되므로 시작하지 않는다
    const HRESULT hr = directQueue->GetQueue()->GetTimestampFrequency(&m_timestampFreq);
    if (FAILED(hr) || m_timestampFreq == 0)
    {
        Core::Logger::Error("FrameProfiler::Initialize: 타임스탬프 주파수를 얻지 못했다 (hr={:#010x}, freq={})",
            static_cast<std::uint32_t>(hr), m_timestampFreq);
        return false;
    }

    m_queryHeap = device.CreateQueryHeap(
        D3D12_QUERY_HEAP_TYPE_TIMESTAMP,
        FrameBufferCount * QueriesPerFrame);
    if (!m_queryHeap) return false;

    const UINT64 size = sizeof(std::uint64_t) * FrameBufferCount * QueriesPerFrame;
    m_readbackResource = resFactory.CreateResource(size, ResInitType::Readback);
    if (!m_readbackResource) return false;

    m_cmdScheduler = &cmdScheduler;
    return true;
}

void FrameProfiler::BeginFrame(CommandList& cmd, std::uint64_t frameIndex)
{
    m_currentSlot = static_cast<UINT>(frameIndex % FrameBufferCount);

    cmd->EndQuery(
        m_queryHeap.Get(),
        D3D12_QUERY_TYPE_TIMESTAMP,
        m_currentSlot * QueriesPerFrame);

    m_cpuStart = std::chrono::high_resolution_clock::now();

    // 주의: frameIndex는 프레임이 성공적으로 끝났을 때만(EndFrame까지 도달했을 때만)
    // 증가해야 함. 실패/abort 시 같은 frameIndex로 재시도되어야 이 슬롯이 안전하게 재사용됨.
}

void FrameProfiler::EndFrame(CommandList& cmd)
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

void FrameProfiler::MarkSubmitted(FenceID fence)
{
    m_slotFences[m_currentSlot] = fence;
}

// 이전 프레임의 칸이 GPU에서 끝나지 않았거나 제출한 적이 없으면 읽지 않고 이전 값을 유지한다. 값이 뒤집힌 칸도 마찬가지다.
void FrameProfiler::Update(std::uint64_t frameIndex)
{
    if (!m_cmdScheduler) return; // 시작하지 못한 프로파일러

    const auto slot = static_cast<UINT>((frameIndex + FrameBufferCount - 1) % FrameBufferCount); // 이전 프레임의 GPU 측정 결과를 읽는다.

    const FenceID fence = m_slotFences[slot];
    if (fence == InvalidFenceID || !m_cmdScheduler->IsFenceComplete(CommandType::Direct, fence))
        return;

    std::uint64_t* data = nullptr;

    D3D12_RANGE range =
    {
        slot * sizeof(std::uint64_t) * QueriesPerFrame,
        (slot + 1) * sizeof(std::uint64_t) * QueriesPerFrame
    };

    const HRESULT hr = m_readbackResource->Map(0, &range, reinterpret_cast<void**>(&data));
    if (FAILED(hr))
    {
        Core::Logger::Error("FrameProfiler::Update: 읽기 버퍼를 매핑하지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return;
    }

    // Map은 range를 읽기 범위로만 알려 주고 포인터는 버퍼 처음을 가리키므로 칸 위치를 더해 읽는다.
    const UINT queryIndex = slot * QueriesPerFrame;

    const std::uint64_t start = data[queryIndex];
    const std::uint64_t end = data[queryIndex + 1];
    if (end >= start)
        m_gpuFrameTimeMs = float(end - start) * 1000.0f / float(m_timestampFreq);

    m_readbackResource->Unmap(0, nullptr);
}
