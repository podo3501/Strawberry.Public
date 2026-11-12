module;

#include <d3d12.h>

module DxRender.Graph;

import :BarrierBuilder;
import :Pass;
import :Definitions;
import Core.Assert;
import DxRender.Command;
import DxRender.Helper;
import DxRender.Task;

static D3D12_RESOURCE_STATES AccessToState(CommandType cmdType, RGAccess access)
{
    if (access == RGAccess::CopyDest)
    {
        if (cmdType == CommandType::Copy)
            return D3D12_RESOURCE_STATE_COMMON; // copy queue 일때에는 common에서 처리하기 때문이다.

        return D3D12_RESOURCE_STATE_COPY_DEST;
    }

    return ToD3D12(access);
}

static CommandType ResolveCommandType(CommandType type,
    D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    Core::Assert(type != CommandType::None);

    // 복사 큐는 COMMON과 복사 상태만 다룰 수 있어서 그 밖의 상태가 끼면 직접 큐가 처리한다.
    auto isCopyQueueState = [](D3D12_RESOURCE_STATES s) {
        return
            s == D3D12_RESOURCE_STATE_COMMON ||
            s == D3D12_RESOURCE_STATE_COPY_DEST ||
            s == D3D12_RESOURCE_STATE_COPY_SOURCE;
        };

    if (type == CommandType::Copy)
        return (isCopyQueueState(before) && isCopyQueueState(after)) ? CommandType::Copy : CommandType::Direct;

    auto isDirectOnly = [](D3D12_RESOURCE_STATES s) {
        return
            (s & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) ||
            (s & D3D12_RESOURCE_STATE_RENDER_TARGET) ||
            (s & D3D12_RESOURCE_STATE_DEPTH_WRITE) ||
            (s & D3D12_RESOURCE_STATE_DEPTH_READ) ||
            (s & D3D12_RESOURCE_STATE_RESOLVE_DEST) ||
            (s & D3D12_RESOURCE_STATE_RESOLVE_SOURCE) ||
            (s & D3D12_RESOURCE_STATE_PRESENT) ||
            (s & D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER) ||
            (s & D3D12_RESOURCE_STATE_INDEX_BUFFER) ||
            (s & D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
        };

    if (isDirectOnly(before) || isDirectOnly(after))
        return CommandType::Direct;

    return type;
}

D3D12_RESOURCE_STATES ToD3D12(RGAccess access)
{
    switch (access)
    {
    case RGAccess::CopyDest:   return D3D12_RESOURCE_STATE_COMMON; // 가져오는 리소스는 만든 직후라 COMMON이다.
    case RGAccess::SRV:        return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    case RGAccess::UAV:        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case RGAccess::RTV:        return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case RGAccess::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case RGAccess::DepthRead:  return D3D12_RESOURCE_STATE_DEPTH_READ;
    case RGAccess::Present:    return D3D12_RESOURCE_STATE_PRESENT;
    }

    return D3D12_RESOURCE_STATE_COMMON; // 정의되지 않은 값
}

BarrierGroups BuildBarriers(
    CommandType cmdType,
    const RenderPass& pass,
    std::unordered_map<RGResourceID, ResourceStateTracker>& stateTracker,
    PassIndex passIndex)
{
    BarrierGroups groups;

    for (auto& usage : pass.usages)
    {
        auto& tracker = stateTracker[usage.resID];
        auto desired = AccessToState(cmdType, usage.state);

        if (tracker.state != desired)
        {
            auto barrierType = ResolveCommandType(cmdType, tracker.state, desired);
            groups[barrierType].push_back({ usage.resID, tracker.state, desired });

            tracker.state = desired;
            tracker.lastUpdatedPass = passIndex;
        }
    }

    return groups;
}

TaskNode CreateBarrierTask(CommandType type, const std::vector<BarrierPlan>& barriers)
{
    TaskNode task{};
    task.passName = "Barrier";
    task.type = type;
    task.execute =
        [
            barriers
        ]
        (TaskCommandLists cmds, TaskContext& ctx)
        {
            CommandList& cmd = cmds.Single();

            std::vector<D3D12_RESOURCE_BARRIER> barrierBatch;
            barrierBatch.reserve(barriers.size());

            for (auto& barrier : barriers)
            {
                auto& res = ctx.GetResource(barrier.resID);

                barrierBatch.push_back(
                    CommandUtils::CreateTransitionBarrier(res, barrier.before, barrier.after));
            }

            CommandUtils::Transition(cmd, barrierBatch);
        };

    return task;
}