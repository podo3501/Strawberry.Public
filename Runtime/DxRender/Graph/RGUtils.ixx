export module DxRender.Graph:Utils;

import std;
import :Definitions;
import Core.Assert;
import DxRender.Task;

export std::vector<PassIndex> TopologicalSort(const std::vector<PassNode>& graph)
{
    const int n = static_cast<int>(graph.size());

    std::vector<int> indegree(n);

    for (int i = 0; i < n; ++i) // indegree 복사
        indegree[i] = graph[i].indegree;

    std::queue<PassIndex> q;
    std::vector<PassIndex> result;
    result.reserve(n);

    // indegree 0부터 시작
    for (PassIndex i = 0; i < n; ++i)
    {
        if (indegree[i] == 0)
            q.push(i);
    }

    while (!q.empty())
    {
        PassIndex cur = q.front();
        q.pop();
        result.push_back(cur);

        for (PassIndex nextPass : graph[cur].dependents)
        {
            indegree[nextPass]--;
            if (indegree[nextPass] == 0)
                q.push(nextPass);
        }
    }

    Core::Assert(result.size() == graph.size()); // Dependency graph contains cycle.

    return result;
}

// 태스크 역방향 의존성(dependents) 빌드
export void BuildDependents(std::vector<CompiledTask>& tasks)
{
    std::unordered_map<LocalTaskID, std::size_t> indexMap;
    indexMap.reserve(tasks.size());

    for (std::size_t i = 0; i < tasks.size(); ++i)
        indexMap[tasks[i].localId] = i;

    for (const auto& task : tasks)
    {
        for (const auto& dep : task.dependencies)
        {
            auto it = indexMap.find(dep);
            Core::Assert(it != indexMap.end()); // 디펜던시가 존재하는데 task가 없다.

            tasks[it->second].dependents.push_back(task.localId);
        }
    }
}

// std::ranges 기반 vector 중복 제거 템플릿 함수
export template <typename T>
    void RemoveVectorDuplicates(std::vector<T>& vec)
{
    std::ranges::sort(vec);
    auto [first, last] = std::ranges::unique(vec); // std::ranges::unique는 중복이 제거된 뒤 남은 '지워야 할 구간'을 subrange로 반환함

    vec.erase(first, last);
}