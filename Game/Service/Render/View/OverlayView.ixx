export module Service.Render:OverlayView;

import std;
import :RenderView;
import Contract.Render.Shared.View;
import Contract.Render.Service;
import Contract.Render.Backend.View;
import Core.Math;

export class OverlayView : public RenderView, public IOverlayView
{
public:
    ~OverlayView() override = default;
    OverlayView(
        RepositoryContainer& repositories,
        MeshHandle uiQuad,
        BrushHandle defaultBrush);
    bool IsEmpty() const override;

    void Reset(const OverlayViewContext& context);
    OverlayViewData TakeData();

    void DrawImage(
        BrushHandle bh,
        const Core::Rect& dest,
        const Core::Rect* source) override;

    void DrawText(
        FontHandle hF,
        TextRenderMode mode,
        std::string_view text,
        std::uint32_t size,
        const Core::Rect& bounds,
        const TextLayout& layout,
        const TextStyle& style) override;

    void DrawText(
        FontHandle hF,
        TextRenderMode mode,
        std::span<const TextSpan> spans,
        std::uint32_t size,
        const Core::Rect& bounds,
        const TextLayout& layout) override;

private:
    MeshHandle m_uiQuad;
    BrushHandle m_defaultBrush;
    OverlayViewData m_data;
};