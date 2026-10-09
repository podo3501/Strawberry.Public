export module DxRender.Packet:RenderSortKey;

import std;

export class RenderSortKey
{
public:
    static std::uint64_t Build(std::uint64_t pso)
    {
        return pso; // 지금은 pso만으로 정렬 하지만 나중에는 정렬 하는 것들이 많이 생긴다.
    }
};