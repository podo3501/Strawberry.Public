module Game.UserInterface.Widget;

import std;
import :ProgressBar;

void ProgressBar::DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const
{
    DrawGauge(overlay, m_skin, screenRect, m_ratio, m_direction);
}