export module Game.UserInterface.Widget:Gauge;

import std;
import :LayoutTypes;
import :SlicedBrush;
import Contract.Render.UI;
import Core.Math;

export struct GaugeSkin
{
    SlicedBrush background;
    SlicedBrush fill;
    Padding fillInset{};   // 배경 안쪽에서 채움이 들어갈 영역의 여백
};

export enum class GaugeDirection { LeftToRight, BottomToTop };

// 게이지를 그린다: 9슬라이스 배경 + 비율(0~1)만큼의 채움. ProgressBar와 Slider가 함께 쓴다.
export void DrawGauge(IOverlayView& overlay, const GaugeSkin& skin, const Core::Rect& rect,
    float ratio, GaugeDirection direction);