module;

#include <d3d12.h>

export module DxRender.Graph:BarrierBuilder;

import std;
import :Pass;
import :Types;
import :Definitions;
import DxRender.Task;
import DxRender.RGResourceID;

export D3D12_RESOURCE_STATES ToD3D12(RGAccess access);

export BarrierGroups BuildBarriers(
    CommandType cmdType,
    const RenderPass& pass,
    std::unordered_map<RGResourceID, ResourceStateTracker>& stateTracker,
    PassIndex passIndex);

export TaskNode CreateBarrierTask(CommandType type, const std::vector<BarrierPlan>& barriers);