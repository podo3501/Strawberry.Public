export module  DxRender.Definition:UIBatchBuffer;

import std;
import Contract.Render.Definition;

export struct UIBatchBuffer
{
    std::vector<UIVertex> vertices;
    std::vector<std::uint32_t> indices;
};