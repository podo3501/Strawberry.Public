module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :ProgressBar;
import Serialization;
import Core.Utils;

ProgressBar::ProgressBar(Core::Rect localRect,
    GaugeSkin skin,
    float ratio,
    GaugeDirection direction)
    : TypeNode(localRect)
    , m_skin(std::move(skin))
    , m_ratio(Clamp01(ratio))
    , m_direction(direction)
{}

void ProgressBar::DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const
{
    DrawGauge(overlay, m_skin, screenRect, m_ratio, m_direction);
}

// 저장 파일에 적히는 이름. 한 번 정하면 바꾸지 않는다 (옛 파일을 읽을 수 없게 된다)
template<>
constexpr std::array<const char*, Core::EnumSize<GaugeDirection>> Core::EnumToStringMap<GaugeDirection>()
{
    return Core::MakeEnumStringMap<GaugeDirection>("LeftToRight", "BottomToTop");
}

bool ProgressBar::Serialize(Serializer& serializer, IWidgetResources& resources)
{
    if (!Widget::Serialize(serializer, resources))
        return false;

    serializer.Process("ratio", m_ratio);
    serializer.Process("direction", m_direction);
    if (m_direction == Core::InvalidEnum<GaugeDirection>)
        return false;
    if (!SerializeGaugeSkin(serializer, resources, m_skin, ""))
        return false;

    if (!serializer.IsWrite())
        m_ratio = Clamp01(m_ratio);   // 파일에 범위를 벗어난 값이 있어도 0~1로 맞춘다
    return true;
}
