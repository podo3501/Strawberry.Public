export module UserInterface.Widget:Gauge;

import std;
import :LayoutTypes;
import :SlicedBrush;
import :WidgetResources;
import Contract.Render.UI;
import Serialization;
import Core.Math;

export struct GaugeSkin
{
    SlicedBrush background;
    SlicedBrush fill;
    Padding fillInset{};   // 배경 안쪽에서 채움이 들어갈 영역의 여백
};

export enum class GaugeDirection { LeftToRight, BottomToTop, Count };   // Count는 저장용 이름 표(ProgressBar.cpp)가 쓴다

// 게이지 비율은 0~1이다. ProgressBar와 Slider가 같은 규칙을 쓰도록 여기에 둔다.
export constexpr float Clamp01(float v) noexcept { return std::clamp(v, 0.f, 1.f); }

// 게이지를 그린다: 9슬라이스 배경 + 비율(0~1)만큼의 채움. ProgressBar와 Slider가 함께 쓴다.
export void DrawGauge(IOverlayView& overlay, const GaugeSkin& skin, const Core::Rect& rect,
    float ratio, GaugeDirection direction);

// 저장/불러오기: 배경, 채움 브러시와 채움 여백을 prefix가 붙은 키로 적는다. 구현: Gauge.cpp
// (prefix가 비어 있으면 backgroundBrush, fillBrush, fillInsetLeft ... 처럼 적힌다)
// 브러시 이름을 모르거나 없으면 false.
export bool SerializeGaugeSkin(Serializer& serializer, IWidgetResources& resources, GaugeSkin& skin, std::string_view prefix);
