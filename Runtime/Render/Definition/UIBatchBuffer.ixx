export module  Runtime.Render.Definition:UIBatchBuffer;

import std;
import Client.Render.Definition;

export struct UIBatchBuffer
{
    std::vector<UIVertex> vertices;
    std::vector<std::uint32_t> indices;
};