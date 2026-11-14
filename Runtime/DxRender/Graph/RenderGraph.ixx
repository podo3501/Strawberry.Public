export module DxRender.Graph:RenderGraph;

import std;
import :Types;
import :Definitions;
import :Pass;
import DxRender.Task;
import DxRender.Command;
import DxRender.RGResourceID;

// 패스와 리소스 상태를 모아 의존과 배리어를 계산해 태스크 목록으로 컴파일한다.
// 매번 Reset한 뒤 쓰고 Compile은 한 번만 부른다. 잘못된 그래프는 Error를 남기고 빈 목록을 돌려준다.
export class RenderGraph
{
public:
    ~RenderGraph();
    RenderGraph();

    void ImportResource(RGResourceID resID, RGAccess access); // 초기 상태 등록
    void ExportResource(RGResourceID resID, RGAccess access); // 최종 완료 상태 등록. Import하지 않았으면 Error를 남기고 무시

    RenderPass& AddGraphicsPass(std::string name);
    RenderPass& AddCopyPass(std::string name);
    RenderPass& AddComputePass(std::string name);

    std::vector<CompiledTask> Compile();
    void Reset();

private:
    struct PlannedBarrier
    {
        BarrierGroups groups;
        LocalTaskID generatedTaskId{ InvalidLocalTaskID }; // 이 배리어가 끝나는 태스크 (배리어 태스크, 패스에 넣었으면 그 패스 태스크)
    };
    using BarrierMap = std::unordered_map<PassIndex, std::vector<std::shared_ptr<PlannedBarrier>>>;

    struct ExportResourceState
    {
        RGResourceID resID;
        RGAccess access;
    };

    RenderPass& AddPass(std::string name, CommandType type);
    bool ValidateGraph() const;
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
        std::vector<CompiledTask>& outTasks,
        PlannedBarrier*& outInPassBarrier);

    LocalTaskID CreateLocalTaskID();

    std::deque<RenderPass> m_passes; // AddPass가 돌려준 참조가 다음 AddPass에서도 유지되게 deque를 쓴다
    std::unordered_map<RGResourceID, ResourceStateTracker> m_statesTracker;
    std::vector<ExportResourceState> m_exportResources;
    LocalTaskID m_localTaskID{ 0 };
};