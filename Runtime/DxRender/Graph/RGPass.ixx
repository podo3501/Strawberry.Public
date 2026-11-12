module;

#include <d3d12.h>

export module DxRender.Graph:Pass;

import std;
import :Types;
import Core.Assert;
import DxRender.RGResourceID;
import DxRender.Command;
import DxRender.Task;

export enum class AccessType
{
    Read,
    Write
};

export struct RGUsage
{
    RGResourceID resID{ InvalidRGID };
    AccessType access{ AccessType::Read };
    RGAccess state{ RGAccess::SRV };
};

export struct BarrierPlan
{
    RGResourceID resID{ InvalidRGID };

    D3D12_RESOURCE_STATES before{ D3D12_RESOURCE_STATE_COMMON };
    D3D12_RESOURCE_STATES after{ D3D12_RESOURCE_STATE_COMMON };
};

export using BarrierGroups = std::unordered_map<CommandType, std::vector<BarrierPlan>>;

export struct RenderPass
{
    std::string name;
    CommandType type{ CommandType::Direct };
    std::uint32_t numParallel{ 1 };
    std::vector<RGUsage> usages;
    std::function<void(TaskCommandLists, TaskContext&)> execute;

    void Read(RGResourceID resID, RGAccess s) { AddUsage(resID, AccessType::Read, s); }
    void Write(RGResourceID resID, RGAccess s) { AddUsage(resID, AccessType::Write, s); }

private:
    // 같은 리소스를 같은 접근과 상태로 다시 등록하면 한 번만 남긴다. 접근이나 상태가 다르면 호출하는 쪽의 실수다.
    void AddUsage(RGResourceID resID, AccessType access, RGAccess state)
    {
        for (const auto& u : usages)
        {
            if (u.resID != resID)
                continue;

            Core::Assert(u.access == access && u.state == state);
            return;
        }
        usages.push_back({ resID, access, state });
    }
};