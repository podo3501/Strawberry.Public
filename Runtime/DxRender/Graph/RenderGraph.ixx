export module DxRender.Graph:RenderGraph;

import std;
import :Types;
import :Definitions;
import :Pass;
import DxRender.Task;
import DxRender.Command;
import DxRender.RGResourceID;

export class RenderGraph
{
public:
    ~RenderGraph();
    RenderGraph();

    void ImportResource(RGResourceID resID, RGAccess access); // 초기 상태 등록
    void ExportResource(RGResourceID resID, RGAccess access); // 최종 완료 상태 등록

    RenderPass& AddGraphicsPass(std::string name);
    RenderPass& AddCopyPass(std::string name);
    RenderPass& AddComputePass(std::string name);

    std::vector<CompiledTask> Compile();
    void Reset();

private:
    struct PlannedBarrier
    {
        BarrierGroups groups;
        LocalTaskID generatedTaskId{ InvalidLocalTaskID }; // 생성된 배리어 태스크 ID
    };
    using BarrierMap = std::unordered_map<PassIndex, std::vector<std::shared_ptr<PlannedBarrier>>>;

    struct ExportResourceState
    {
        RGResourceID resID;
        RGAccess access;
    };

    RenderPass& AddPass(std::string name, CommandType type);
    void ValidateGraph();
    void BuildExportPass();
    std::vector<PassNode> BuildDependencyGraph();
    BarrierMap PlanBarriers(const std::vector<PassIndex>& sortedPass);

    std::vector<CompiledTask> BuildCompiledTasks(
        const std::vector<PassNode>& passNodes,
        const std::vector<PassIndex>& sortedPass,
        const BarrierMap& passToBarriersMap);

    std::vector<LocalTaskID> BuildBarrierTasks(
        PassIndex passIndex,
        const std::vector<LocalTaskID>& baseDependencies,
        const BarrierMap& passToBarriersMap,
        std::vector<CompiledTask>& outTasks);

    LocalTaskID CreateLocalTaskID();

    std::vector<RenderPass> m_passes;
    std::unordered_map<RGResourceID, ResourceStateTracker> m_statesTracker;
    std::vector<ExportResourceState> m_exportResources;
    LocalTaskID m_localTaskID{ 0 };
};