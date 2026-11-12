module;

#include <d3d12.h>

export module DxRender.Graph:Definitions;

import std;

export using PassIndex = int;

export struct PassNode
{
    PassIndex index{ -1 };
    std::vector<PassIndex> dependencies; // 내 앞에 실행되어야 하는 패스들
    std::vector<PassIndex> dependents;   // 내 뒤에 실행되어야 하는 패스들
    int indegree{ 0 };
};

export struct ResourceStateTracker //리소스 상태 추적기
{
    D3D12_RESOURCE_STATES state{ D3D12_RESOURCE_STATE_COMMON };
    PassIndex lastUpdatedPass{ -1 };
};