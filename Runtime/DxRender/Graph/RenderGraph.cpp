module DxRender.Graph;

import std;
import :Pass;
import :Utils;
import :Types;
import :BarrierBuilder;
import Core.Assert;
import DxRender.RGResourceID;
import DxRender.Command;

RenderGraph::~RenderGraph() = default;
RenderGraph::RenderGraph() = default;

void RenderGraph::ImportResource(RGResourceID resID, RGAccess access)
{
    m_statesTracker[resID].state = ToD3D12(access);
}

void RenderGraph::ExportResource(RGResourceID resID, RGAccess access)
{
    Core::Assert(m_statesTracker.contains(resID)); // 초기 상태가 존재해야 함
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
    ValidateGraph();

    BuildExportPass();
    auto passNodes = BuildDependencyGraph();                 // 종속성 그래프 생성
    auto sortedPass = TopologicalSort(passNodes);       // 위상 정렬
    auto barrierMap = PlanBarriers(sortedPass);              // 배리어 계획 수립

    auto tasks = BuildCompiledTasks(passNodes, sortedPass, barrierMap);
    BuildDependents(tasks);
    Core::Assert(!tasks.empty());                            // 태스크 집합 검증

    return tasks;
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

        std::vector<LocalTaskID> currentPassDependencies =
            BuildBarrierTasks(passIndex, baseDependencies, passToBarriersMap, tasks);

        if (!pass.execute)
            continue;

        TaskNode task{};
        task.passName = pass.name;
        task.type = pass.type;
        task.numParallel = pass.numParallel;
        task.execute = pass.execute;

        LocalTaskID taskId = CreateLocalTaskID();
        tasks.push_back({ taskId, std::move(task), currentPassDependencies });

        passToTaskId[passIndex] = taskId;
    }

    return tasks;
}

std::vector<LocalTaskID> RenderGraph::BuildBarrierTasks(
    PassIndex passIndex,
    const std::vector<LocalTaskID>& baseDependencies,
    const BarrierMap& passToBarriersMap,
    std::vector<CompiledTask>& outTasks)
{
    auto barrierIt = passToBarriersMap.find(passIndex);
    if (barrierIt == passToBarriersMap.end() || barrierIt->second.empty())
        return baseDependencies; // 배리어가 없으면 부모 의존성을 그대로 반환

    std::vector<LocalTaskID> currentPassDependencies;
    currentPassDependencies.reserve(barrierIt->second.size());

    for (auto& planned : barrierIt->second)
    {
        if (planned->generatedTaskId == InvalidLocalTaskID)
        {
            std::vector<LocalTaskID> barrierDependencies = baseDependencies;
            for (auto& [type, barriers] : planned->groups)
            {
                auto barrierTask = CreateBarrierTask(type, barriers);
                LocalTaskID barrierId = CreateLocalTaskID();
                outTasks.push_back({ barrierId, std::move(barrierTask), std::move(barrierDependencies) });

                barrierDependencies.clear();
                barrierDependencies.push_back(barrierId); // 체이닝: 다음 그룹 배리어는 방금 생성한 배리어 태스크에 의존
            }
            planned->generatedTaskId = barrierDependencies.back();
        }
        currentPassDependencies.push_back(planned->generatedTaskId);
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

void RenderGraph::ValidateGraph()
{
    std::unordered_set<RGResourceID> produced;

    for (auto& [resID, state] : m_statesTracker)
        produced.insert(resID);

    for (auto& pass : m_passes)
    {
        for (auto& usage : pass.usages)
        {
            if (usage.access == AccessType::Read)
            {
                Core::Assert(produced.contains(usage.resID));
            }

            if (usage.access == AccessType::Write)
            {
                produced.insert(usage.resID);
            }
        }
    }
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

        if (!barrierGroups.empty())
        {
            auto planned = std::make_shared<PlannedBarrier>();
            planned->groups = std::move(barrierGroups);

            passToBarriersMap[passIndex].push_back(planned);
            for (const auto& usage : pass.usages)
            {
                if (tempTracker[usage.resID].lastUpdatedPass == passIndex)
                    lastResourceBarrier[usage.resID] = planned;
            }
        }
        else
        {
            for (const auto& usage : pass.usages)
            {
                if (usage.access == AccessType::Read && lastResourceBarrier.contains(usage.resID))
                    passToBarriersMap[passIndex].push_back(lastResourceBarrier[usage.resID]);
            }
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