export module DxRender.Definition:RenderData;

import std;
import :RenderState;
import Core.Math;
import DxRender.RGResourceID;
import Contract.Render.Definition;
import Contract.Render.View;
import Contract.Render.IResource;

export struct RenderSurfaceItem
{
    std::shared_ptr<IResource> mesh;
    std::shared_ptr<IResource> material;
    std::optional<ShaderID> shaderOverride;
    Core::Matrix world{};

    std::uint64_t sortKey{ 0 };
    PipelineState pipelineState{};
};

export struct RenderDebugSurfaceItem
{
    std::shared_ptr<IResource> mesh;
    std::shared_ptr<IResource> material;
    Core::Matrix world{};

    std::uint64_t sortKey{ 0 };
};

export struct RenderUIItem
{
    std::shared_ptr<IResource> mesh;
};

export struct RenderTextItem
{
    std::shared_ptr<IResource> fontRes;
    TextRenderMode mode;
    std::uint32_t fontSize{ 0 };
    Core::Vector2 position{};
    Core::Vector2 size{};
    TextLayout layout{};
    std::vector<TextRun> runs;
};

export struct ViewTargetPacket
{
    ViewID id;
    CameraData camera;
    Core::Rect viewport;      // 화면(백버퍼) 상의 배치. Composite가 사용
    Core::Rect localViewport; // 뷰 타겟 텍스처 내부 좌표(0,0 시작)
};

export struct SceneViewPacket
{
    ViewTargetPacket target;

    std::vector<RenderSurfaceItem> surface;
    std::vector<RenderDebugSurfaceItem> debugSurface;
    std::shared_ptr<IResource> environment{ nullptr }; // nullptr 가능 - 환경 없는 씬
};

export struct OverlayViewPacket
{
    ViewTargetPacket target;

    std::optional<RenderUIItem> ui;
};

export struct RenderShadowCasterItem
{
    std::shared_ptr<IResource> mesh;
    Core::Matrix world;
};

export struct FramePacket
{
    DirectionalLightData light;
    std::vector<RenderShadowCasterItem> shadowCasters;
    std::vector<std::shared_ptr<SceneViewPacket>> sceneViews;
    std::vector<std::shared_ptr<OverlayViewPacket>> overlayViews;
};

export struct ViewRenderOutput
{
    ViewID id;
    Core::Rect viewport;
    std::uint32_t heapIndex;
    RGResourceID colorID;
};