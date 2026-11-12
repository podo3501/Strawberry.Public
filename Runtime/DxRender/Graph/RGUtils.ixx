export module DxRender.Graph:Utils;

import std;
import :Definitions;
import Core.Assert;

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

// std::ranges 기반 vector 중복 제거 템플릿 함수
export template <typename T>
    void RemoveVectorDuplicates(std::vector<T>& vec)
{
    std::ranges::sort(vec);
    auto [first, last] = std::ranges::unique(vec); // std::ranges::unique는 중복이 제거된 뒤 남은 '지워야 할 구간'을 subrange로 반환함

    vec.erase(first, last);
}