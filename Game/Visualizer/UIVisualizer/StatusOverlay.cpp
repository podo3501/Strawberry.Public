module UIVisualizer.StatusOverlay;

import std;

void StatusOverlay::Draw(IOverlayView& overlay, const Core::Size& screen, bool visualMode, std::string_view error) const
{
    if (!m_font)
        return;

    const Core::Rect bounds{ kMargin, kMargin,
        static_cast<float>(screen.width) - kMargin * 2.f, static_cast<float>(screen.height) - kMargin * 2.f };
    if (!bounds.IsValid())
        return;

    if (!error.empty())
    {
        TextStyle style;
        style.color = kErrorColor;
        overlay.DrawText(m_font, TextRenderMode::MTSDF, error, kFontSize, bounds, TextLayout{}, style);
    }

    if (visualMode)
    {
        TextLayout layout;
        layout.horizontalAlign = TextHorizontalAlign::Right;
        layout.verticalAlign = TextVerticalAlign::Bottom;
        TextStyle style;
        style.color = kModeColor;
        overlay.DrawText(m_font, TextRenderMode::MTSDF, kModeLabel, kFontSize, bounds, layout, style);
    }
}
