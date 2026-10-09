export module DxRender.Packet:PacketBuilder;

import std;
import :SurfaceItemBuilder;
import :DebugSurfaceItemBuilder;
import :UIItemBuilder;
import DxRender.Definition;
import DxRender.Text;
import DxRender.Provider;
import Core.Math;
import Contract.Render.View;

namespace
{
    Core::Rect ResolveViewport(const std::optional<Core::Rect>& requestedViewport, const Core::Size& screenSize)
    {
        return requestedViewport.value_or(
            Core::Rect{ 0.f, 0.f, static_cast<float>(screenSize.width), static_cast<float>(screenSize.height) });
    }

    void FillTarget(
        ViewTargetPacket& target,
        const ViewTargetInfo& targetInfo,
        const Core::Size& screenSize)
    {
        target.id = targetInfo.id;
        target.camera = targetInfo.camera;
        target.viewport = ResolveViewport(targetInfo.viewport, screenSize);
        target.localViewport = Core::Rect{ 0.f, 0.f, target.viewport.width, target.viewport.height };
    }

    std::shared_ptr<SceneViewPacket> BuildSceneViewPacket(
        SceneViewData&& view,
        const Core::Size& screenSize)
    {
        auto packet = std::make_shared<SceneViewPacket>();
        FillTarget(packet->target, view.context.target, screenSize);

        if (view.draws.environment) packet->environment = view.draws.environment;
        packet->surface = BuildSurfaceItems(view.draws.surfaces, view.context.renderOverride.rasterPreset);
        packet->debugSurface = BuildDebugSurfaceItems(view.draws.debugSurfaces);

        return packet;
    }

    std::shared_ptr<OverlayViewPacket> BuildOverlayViewPacket(
        OverlayViewData&& view,
        TextSystem& textSystem,
        TransientMeshProvider& meshProvider,
        const Core::Size& screenSize)
    {
        auto packet = std::make_shared<OverlayViewPacket>();
        FillTarget(packet->target, view.context.target, screenSize);

        packet->ui = BuildUIItems(view.draws, textSystem, meshProvider);
        return packet;
    }

    std::vector<std::shared_ptr<SceneViewPacket>> BuildSceneViews(
        std::vector<SceneViewData>& views,
        const Core::Size& screenSize)
    {
        std::vector<std::shared_ptr<SceneViewPacket>> result;
        result.reserve(views.size());

        for (auto& view : views)
            result.push_back(BuildSceneViewPacket(std::move(view), screenSize));

        return result;
    }

    std::vector<std::shared_ptr<OverlayViewPacket>> BuildOverlayViews(
        std::vector<OverlayViewData>& views,
        TextSystem& textSystem,
        TransientMeshProvider& meshProvider,
        const Core::Size& screenSize)
    {
        std::vector<std::shared_ptr<OverlayViewPacket>> result;
        result.reserve(views.size());

        for (auto& view : views)
            result.push_back(BuildOverlayViewPacket(std::move(view), textSystem, meshProvider, screenSize));

        return result;
    }

    std::vector<RenderShadowCasterItem> BuildShadowCasters(
        std::vector<DrawShadowCasterItem>& casters)
    {
        std::vector<RenderShadowCasterItem> result;
        result.reserve(casters.size());

        for (auto& caster : casters)
        {
            result.push_back(
                RenderShadowCasterItem{
                    std::move(caster.mesh),
                    caster.world
                });
        }

        return result;
    }
} //namespace

export FramePacket BuildPacket(
    SceneFrameData& frame,
    TextSystem& textSystem,
    TransientMeshProvider& meshProvider,
    const Core::Size& screenSize)
{
    FramePacket packet;
    packet.light = std::move(frame.light);
    packet.shadowCasters = BuildShadowCasters(frame.shadowCasters);
    packet.sceneViews = BuildSceneViews(frame.sceneViews, screenSize);
    packet.overlayViews = BuildOverlayViews(frame.overlayViews, textSystem, meshProvider, screenSize);

    return packet;
}