export module DxRender.Task:CompiledNode;

import std;
import :Types;
import :Node;

// RenderGraph에서 pass를 가지고 계산해서 tasks로 만든 결과물.
export struct CompiledTask
{
    LocalTaskID localId{ InvalidLocalTaskID };
    TaskNode task{};
    std::vector<std::uint32_t> dependencies;
    std::vector<std::uint32_t> dependents;
};