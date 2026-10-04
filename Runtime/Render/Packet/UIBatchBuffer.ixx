export module  Runtime.Render.Packet:UIBatchBuffer;

import std;
import Runtime.Render.Resource;
import Client.Render.Definition;

export struct UIBatchBuffer
{
    std::vector<UIVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::shared_ptr<BrushResource> brush{ nullptr };
};