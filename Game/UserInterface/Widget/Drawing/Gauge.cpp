module Game.UserInterface.Widget;

import std;
import :Gauge;

void DrawGauge(IOverlayView& overlay, const GaugeSkin& skin, const Core::Rect& rect,
    float ratio, GaugeDirection direction)
{
    ratio = Clamp01(ratio);

    DrawSliced(overlay, skin.background, rect);

    const Padding& inset = skin.fillInset;
    const Core::Rect inner{
        rect.x + inset.left,
        rect.y + inset.top,
        (std::max)(0.f, rect.width - inset.Horizontal()),
        (std::max)(0.f, rect.height - inset.Vertical()) };

    // 반 픽셀에 걸려 가장자리가 흐려지지 않도록 채움 길이를 정수로 맞춘다 (Anchor::Resolve와 같은 이유)
    Core::Rect fill = inner;
    if (direction == GaugeDirection::LeftToRight)
    {
        fill.width = std::round(inner.width * ratio);
    }
    else
    {
        fill.height = std::round(inner.height * ratio);
        fill.y = inner.y + inner.height - fill.height;   // 아래에서 위로 차오른다
    }

    if (fill.width <= 0.f || fill.height <= 0.f)
        return;

    DrawSliced(overlay, skin.fill, fill);
}