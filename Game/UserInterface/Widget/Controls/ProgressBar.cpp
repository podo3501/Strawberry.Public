module Game.UserInterface.Widget;

import std;
import :ProgressBar;

ProgressBar::ProgressBar(Core::Rect localRect,
    GaugeSkin skin,
    float ratio,
    GaugeDirection direction)
    : Widget(localRect)
    , m_skin(std::move(skin))
    , m_ratio(Clamp01(ratio))
    , m_direction(direction)
{}

void ProgressBar::DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const
{
    DrawGauge(overlay, m_skin, screenRect, m_ratio, m_direction);
}