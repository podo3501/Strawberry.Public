export module DxRender.Packet:SurfaceItemBuilder;

import std;
import :RenderSortKey;
import DxRender.Definition;
import DxRender.Resource;
import Contract.Render.Shared.Types;
import Contract.Render.Backend.View;
import Core.Logger;
import Core.TypeHierarchy;

// 메시나 재질이 아닌 리소스가 들어오면 Error를 남기고 그리지 않는다 (nullopt)
static std::optional<RenderSurfaceItem> BuildSurfaceItem(
    const DrawSurfaceItem& drawItem,
    std::optional<RasterPreset> rasterOverride)
{
    auto material = Core::Cast<MaterialResource>(drawItem.material);
    if (!Core::Cast<MeshResource>(drawItem.mesh) || !material)
    {
        Core::Logger::Error("BuildSurfaceItem: 메시나 재질이 올바른 리소스가 아니라 그리지 않는다 (메시 {}, 재질 {})",
            Core::Cast<MeshResource>(drawItem.mesh) != nullptr, material != nullptr);
        return std::nullopt;
    }

    RenderSurfaceItem item{
        drawItem.mesh,
        drawItem.material,
        drawItem.shaderOverride,
        drawItem.world };

    PipelineState pso = material->GetPipelineState(rasterOverride, drawItem.shaderOverride);
    item.sortKey = RenderSortKey::Build(pso.GetHash());
    item.pipelineState = pso;

    return item;
}

export std::vector<RenderSurfaceItem> BuildSurfaceItems(
    std::vector<DrawSurfaceItem>& surfaces,
    std::optional<RasterPreset> rasterOverride)
{
    std::vector<RenderSurfaceItem> result;
    result.reserve(surfaces.size());

    for (auto& surface : surfaces)
    {
        if (auto item = BuildSurfaceItem(surface, rasterOverride))
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