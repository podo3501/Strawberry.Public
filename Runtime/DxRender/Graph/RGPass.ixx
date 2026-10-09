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
    RGResourceID resID;
    AccessType access;
    RGAccess state;
};

export struct BarrierPlan
{
    RGResourceID resID;

    D3D12_RESOURCE_STATES before;
    D3D12_RESOURCE_STATES after;
};

export using BarrierGroups = std::unordered_map<CommandType, std::vector<BarrierPlan>>;

export struct RenderPass
{
    std::string name;
    CommandType type{ CommandType::Direct };
    std::uint32_t numParallel{ 1 };
    std::vector<RGUsage> usages;
    std::function<void(TaskCommandLists, TaskContext&)> execute;

    void Read(RGResourceID resID, RGAccess s)
    {
        Core::Assert(!HasUsage(resID)); // 같은 패스에서 리소스를 중복해서 쓰면 안된다.
        usages.push_back({ resID, AccessType::Read, s });
    }

    void Write(RGResourceID resID, RGAccess s)
    {
        Core::Assert(!HasUsage(resID)); // 같은 패스에서 리소스를 중복해서 쓰면 안된다.
        usages.push_back({ resID, AccessType::Write, s });
    }

private:
    bool HasUsage(RGResourceID resID) const
    {
        for (const auto& u : usages)
        {
            if (u.resID == resID)
                return true;
        }
        return false;
    }
};