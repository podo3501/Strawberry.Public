export module DxRender.Packet:DebugSurfaceItemBuilder;

import std;
import :RenderSortKey;
import DxRender.Definition;
import DxRender.Resource;
import Contract.Render.Backend.View;
import Core.Logger;
import Core.TypeHierarchy;

// 메시나 디버그 재질이 아닌 리소스가 들어오면 Error를 남기고 그리지 않는다 (nullopt)
std::optional<RenderDebugSurfaceItem> BuildDebugSurfaceItem(const DrawDebugSurfaceItem& drawItem)
{
    auto debugMaterial = Core::Cast<DebugMaterialResource>(drawItem.material);
    if (!Core::Cast<MeshResource>(drawItem.mesh) || !debugMaterial)
    {
        Core::Logger::Error("BuildDebugSurfaceItem: 메시나 디버그 재질이 올바른 리소스가 아니라 그리지 않는다 (메시 {}, 재질 {})",
            Core::Cast<MeshResource>(drawItem.mesh) != nullptr, debugMaterial != nullptr);
        return std::nullopt;
    }

    RenderDebugSurfaceItem item{
        drawItem.mesh,
        drawItem.material,
        drawItem.world };

    item.sortKey = RenderSortKey::Build(debugMaterial->GetPipelineState().GetHash());

    return item;
}

export std::vector<RenderDebugSurfaceItem> BuildDebugSurfaceItems(
    std::vector<DrawDebugSurfaceItem>& debugSurfaces)
{
    std::vector<RenderDebugSurfaceItem> result;
    result.reserve(debugSurfaces.size());

    for (auto& debugSurface : debugSurfaces)
    {
        if (auto item = BuildDebugSurfaceItem(debugSurface))
            result.push_back(std::move(*item));
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const auto& a, const auto& b)
        {
            return a.sortKey < b.sortKey;
        });

    return result;
}