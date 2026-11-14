module DxRender.Graph;

import std;
import :RenderGraph;
import :Pass;
import :Utils;
import :Types;
import :BarrierBuilder;
import Core.Assert;
import Core.Logger;
import DxRender.RGResourceID;
import DxRender.Command;

namespace
{
    // 한 패스의 배리어를 큐 종류별로 이을 때 쓰는 고정된 순서
    constexpr std::array<CommandType, 3> BarrierQueueOrder{ CommandType::Direct, CommandType::Compute, CommandType::Copy };

    // from 태스크들이 target을 (직접이든 다른 태스크를 거쳐서든) 이미 기다리는지. 태스크 번호는 tasks의 위치와 같다.
    bool AlreadyWaitsFor(const std::vector<CompiledTask>& tasks, const std::vector<LocalTaskID>& from, LocalTaskID target)
    {
        std::vector<LocalTaskID> pending = from;
        std::unordered_set<LocalTaskID> seen;
        while (!pending.empty())
        {
            const LocalTaskID current = pending.back();
            pending.pop_back();

            if (current == target)
                return true;
            if (current >= tasks.size() || !seen.insert(current).second)
                continue;

            pending.insert(pending.end(), tasks[current].dependencies.begin(), tasks[current].dependencies.end());
        }
        return false;
    }
}

RenderGraph::~RenderGraph() = default;
RenderGraph::RenderGraph() = default;

void RenderGraph::ImportResource(RGResourceID resID, RGAccess access)
{
    m_statesTracker[resID].state = ToD3D12(access);
}

void RenderGraph::ExportResource(RGResourceID resID, RGAccess access)
{
    if (!m_statesTracker.contains(resID))
    {
        Core::Logger::Error("RenderGraph::ExportResource: Import하지 않은 리소스다 (id={})", resID);
        return;
    }

    for (const auto& exported : m_exportResources)
    {
        if (exported.resID != resID)
            continue;

        if (exported.access != access)
            Core::Logger::Error("RenderGraph::ExportResource: 이미 다른 접근으로 내보내기로 등록한 리소스다 (id={})", resID);
        return; // 같은 접근이면 한 번만 등록한다
    }

    m_exportResources.emplace_back(resID, access);
}

RenderPass& RenderGraph::AddGraphicsPass(std::string n) { return AddPass(std::move(n), CommandType::Direct); }
RenderPass& RenderGraph::AddCopyPass(std::string n) { return AddPass(std::move(n), CommandType::Copy); }
RenderPass& RenderGraph::AddComputePass(std::string n) { return AddPass(std::move(n), CommandType::Compute); }

RenderPass& RenderGraph::AddPass(std::string name, CommandType type)
{
    m_passes.emplace_back();

    auto& pass = m_passes.back();
    pass.name = std::move(name);
    pass.type = type;

    return pass;
}

std::vector<CompiledTask> RenderGraph::Compile()
{
    if (!ValidateGraph())
        return {};

    BuildExportPass();
    auto passNodes = BuildDependencyGraph();                 // 종속성 그래프 생성
    auto sortedPass = TopologicalSort(passNodes);       // 위상 정렬
    auto barrierMap = PlanBarriers(sortedPass);              // 배리어 계획 수립

    return BuildCompiledTasks(passNodes, sortedPass, barrierMap); // 할 일이 없는 그래프는 빈 목록이다
}

void RenderGraph::Reset()
{
    m_passes.clear();
    m_statesTracker.clear();
    m_exportResources.clear();
    m_localTaskID = 0;
}

void RenderGraph::BuildExportPass()
{
    if (m_exportResources.empty())
        return;

    auto& pass = AddGraphicsPass("__Export_Internal");
    for (auto& [resID, access] : m_exportResources)
        pass.Write(resID, access);
}

std::vector<CompiledTask> RenderGraph::BuildCompiledTasks(
    const std::vector<PassNode>& passNodes,
    const std::vector<PassIndex>& sortedPass,
    const BarrierMap& passToBarriersMap)
{
    std::vector<CompiledTask> tasks;
    tasks.reserve(sortedPass.size() * 2);
    std::unordered_map<PassIndex, LocalTaskID> passToTaskId;
    passToTaskId.reserve(sortedPass.size());

    for (PassIndex passIndex : sortedPass)
    {
        auto& pass = m_passes[passIndex];

        // 부모 패스 의존성 수집
        std::vector<LocalTaskID> baseDependencies;

        for (PassIndex depPass : passNodes[passIndex].dependencies)
        {
            auto it = passToTaskId.find(depPass);
            if (it != passToTaskId.end())
                baseDependencies.push_back(it->second);
        }

        PlannedBarrier* inPassBarrier = nullptr;
        std::vector<LocalTaskID> currentPassDependencies =
            BuildBarrierTasks(passIndex, baseDependencies, passToBarriersMap, tasks, inPassBarrier);

        if (!pass.execute)
            continue;

        TaskNode task{};
        task.passName = pass.name;
        task.type = pass.type;
        task.numParallel = pass.numParallel;
        task.execute = pass.execute;

        if (inPassBarrier)
        {
            task.execute =
                [
                    barrier = CreateBarrierTask(pass.type, inPassBarrier->groups.at(pass.type)).execute,
                    run = pass.execute
                ]
                (TaskCommandLists cmds, TaskContext& ctx)
                {
                    barrier(cmds, ctx);
                    run(cmds, ctx);
                };
        }

        LocalTaskID taskId = CreateLocalTaskID();
        tasks.push_back({ taskId, std::move(task), currentPassDependencies });

        passToTaskId[passIndex] = taskId;
        if (inPassBarrier)
            inPassBarrier->generatedTaskId = taskId; // 이 배리어를 기다리는 다른 패스는 이 패스를 기다린다
    }

    return tasks;
}

// 패스와 큐 종류가 같은 배리어는 태스크로 만들지 않고 outInPassBarrier로 돌려줘서 패스의 명령 목록 맨 앞에 넣게 한다.
// 배리어와 그 리소스를 쓰는 셰이더가 한 명령 목록에 있어야 GPU 기반 검증이 상태를 놓치지 않고, 제출 횟수도 준다.
std::vector<LocalTaskID> RenderGraph::BuildBarrierTasks(
    PassIndex passIndex,
    const std::vector<LocalTaskID>& baseDependencies,
    const BarrierMap& passToBarriersMap,
    std::vector<CompiledTask>& outTasks,
    PlannedBarrier*& outInPassBarrier)
{
    outInPassBarrier = nullptr;

    // 실행 함수가 없는 패스(내보내기)와 명령 목록을 여러 개 쓰는 패스에는 넣을 수 없다.
    const RenderPass& pass = m_passes[passIndex];
    const bool canRunInPass = pass.execute && pass.numParallel <= 1;

    auto barrierIt = passToBarriersMap.find(passIndex);
    if (barrierIt == passToBarriersMap.end() || barrierIt->second.empty())
        return baseDependencies; // 배리어가 없으면 부모 의존성을 그대로 반환

    std::vector<LocalTaskID> barrierIds;
    barrierIds.reserve(barrierIt->second.size());

    for (auto& planned : barrierIt->second)
    {
        if (planned->generatedTaskId == InvalidLocalTaskID)
        {
            std::vector<LocalTaskID> barrierDependencies = baseDependencies;
            LocalTaskID lastBarrierId = InvalidLocalTaskID;
            for (CommandType type : BarrierQueueOrder)
            {
                auto groupIt = planned->groups.find(type);
                if (groupIt == planned->groups.end())
                    continue;

                if (canRunInPass && type == pass.type)
                {
                    outInPassBarrier = planned.get();
                    continue;
                }

                lastBarrierId = CreateLocalTaskID();
                outTasks.push_back({ lastBarrierId, CreateBarrierTask(type, groupIt->second), std::move(barrierDependencies) });

                barrierDependencies = { lastBarrierId }; // 체이닝: 다음 그룹 배리어는 방금 생성한 배리어 태스크에 의존
            }
            planned->generatedTaskId = lastBarrierId;
        }

        if (planned->generatedTaskId != InvalidLocalTaskID) // 전부 패스에 넣었으면 기다릴 배리어 태스크가 없다
            barrierIds.push_back(planned->generatedTaskId);
    }

    // 배리어가 이미 기다리는 앞 패스는 따로 기다리지 않는다. 다른 패스가 만든 배리어를 재사용하면 그 배리어가 기다리지 않는 앞 패스만 남는다.
    std::vector<LocalTaskID> currentPassDependencies = barrierIds;
    for (const LocalTaskID base : baseDependencies)
    {
        if (!AlreadyWaitsFor(outTasks, barrierIds, base))
            currentPassDependencies.push_back(base);
    }
    RemoveVectorDuplicates(currentPassDependencies);

    return currentPassDependencies;
}

std::vector<PassNode> RenderGraph::BuildDependencyGraph()
{
    const int passCount = static_cast<int>(m_passes.size());

    std::vector<PassNode> nodes(passCount);

    for (int i = 0; i < passCount; ++i)
        nodes[i].index = i;

    std::unordered_map<RGResourceID, PassIndex> lastWriter;
    std::unordered_map<RGResourceID, std::vector<PassIndex>> activeReaders;

    for (PassIndex passIndex = 0; passIndex < passCount; ++passIndex)
    {
        auto& pass = m_passes[passIndex];

        for (auto& usage : pass.usages)
        {
            const auto resourceID = usage.resID;

            switch (usage.access)
            {
            case AccessType::Read:
            {
                // RAW(write->read)
                auto writerIt = lastWriter.find(resourceID);
                if (writerIt != lastWriter.end())
                {
                    int writerPass = writerIt->second;

                    nodes[passIndex].dependencies.push_back(writerPass);
                    nodes[writerPass].dependents.push_back(passIndex);
                }

                activeReaders[resourceID].push_back(passIndex);
                break;
            }

            case AccessType::Write:
            {
                // WAW(write->write)
                auto writerIt = lastWriter.find(resourceID);
                if (writerIt != lastWriter.end())
                {
                    int writerPass = writerIt->second;

                    nodes[passIndex].dependencies.push_back(writerPass);
                    nodes[writerPass].dependents.push_back(passIndex);
                }

                // WAR(read->write)
                auto readerIt = activeReaders.find(resourceID);
                if (readerIt != activeReaders.end())
                {
                    for (int readerPass : readerIt->second)
                    {
                        nodes[passIndex].dependencies.push_back(readerPass);
                        nodes[readerPass].dependents.push_back(passIndex);
                    }

                    readerIt->second.clear();
                }

                lastWriter[resourceID] = passIndex;
                break;
            }
            }
        }
    }

    for (auto& node : nodes)
    {
        RemoveVectorDuplicates(node.dependencies);
        RemoveVectorDuplicates(node.dependents);

        node.indegree = static_cast<int>(node.dependencies.size());
    }

    return nodes;
}

bool RenderGraph::ValidateGraph() const
{
    std::unordered_set<RGResourceID> produced;

    for (const auto& [resID, state] : m_statesTracker)
        produced.insert(resID);

    for (const auto& pass : m_passes)
    {
        for (const auto& usage : pass.usages)
        {
            if (usage.access == AccessType::Write)
            {
                produced.insert(usage.resID);
                continue;
            }

            if (!produced.contains(usage.resID))
            {
                Core::Logger::Error("RenderGraph::Compile: 만들어지지 않은 리소스를 읽는다 (pass={}, id={})", pass.name, usage.resID);
                return false;
            }
        }
    }

    return true;
}

RenderGraph::BarrierMap RenderGraph::PlanBarriers(const std::vector<PassIndex>& sortedPass)
{
    std::unordered_map<PassIndex, std::vector<std::shared_ptr<PlannedBarrier>>> passToBarriersMap;
    std::unordered_map<RGResourceID, std::shared_ptr<PlannedBarrier>> lastResourceBarrier;

    auto tempTracker = m_statesTracker;

    for (PassIndex passIndex : sortedPass)
    {
        auto& pass = m_passes[passIndex];
        BarrierGroups barrierGroups = BuildBarriers(pass.type, pass, tempTracker, passIndex);

        // 이 패스가 상태를 바꾸지 않는 읽기는, 그 리소스를 마지막으로 바꾼 배리어(다른 패스가 만든 것)가 끝난 뒤에 실행되어야 한다.
        for (const auto& usage : pass.usages)
        {
            if (usage.access != AccessType::Read || tempTracker[usage.resID].lastUpdatedPass == passIndex)
                continue;

            if (auto it = lastResourceBarrier.find(usage.resID); it != lastResourceBarrier.end())
                passToBarriersMap[passIndex].push_back(it->second);
        }

        if (barrierGroups.empty())
            continue;

        auto planned = std::make_shared<PlannedBarrier>();
        planned->groups = std::move(barrierGroups);

        passToBarriersMap[passIndex].push_back(planned);
        for (const auto& usage : pass.usages)
        {
            if (tempTracker[usage.resID].lastUpdatedPass == passIndex)
                lastResourceBarrier[usage.resID] = planned;
        }
    }

    m_statesTracker = std::move(tempTracker);
    return passToBarriersMap;
}

LocalTaskID RenderGraph::CreateLocalTaskID()
{
    Core::Assert(m_localTaskID != InvalidLocalTaskID);
    return m_localTaskID++;
}