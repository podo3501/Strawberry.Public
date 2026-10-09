export module DxRender.Packet:DebugSurfaceItemBuilder;

import std;
import :RenderSortKey;
import DxRender.Definition;
import DxRender.Resource;
import Contract.Render.View;

RenderDebugSurfaceItem BuildDebugSurfaceItem(const DrawDebugSurfaceItem& drawItem)
{
    RenderDebugSurfaceItem item{
        drawItem.mesh,
        drawItem.material,
        drawItem.world };

    auto debugMaterial = static_cast<DebugMaterialResource*>(drawItem.material.get());
    item.sortKey = RenderSortKey::Build(debugMaterial->GetPipelineState().GetHash());

    return item;
}

export std::vector<RenderDebugSurfaceItem> BuildDebugSurfaceItems(
    std::vector<DrawDebugSurfaceItem>& debugSurfaces)
{
    std::vector<RenderDebugSurfaceItem> result;
    result.reserve(debugSurfaces.size());

    for (auto& debugSurface : debugSurfaces)
        result.push_back(BuildDebugSurfaceItem(debugSurface));

    std::sort(
        result.begin(),
        result.end(),
        [](const auto& a, const auto& b)
        {
            return a.sortKey < b.sortKey;
        });

    return result;
}