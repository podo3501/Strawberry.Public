export module Game.UserInterface.Widget:ProgressBar;

import std;
import :Widget;
import :Gauge;
import Contract.Render.UI;
import Core.Math;

// 왼쪽에서 오른쪽으로 차오르는 게이지. 값은 0~1 비율이다 (HP, 로딩 등).
// 채움도 SlicedBrush라서 9슬라이스로 둥근 끝을 만들 수 있고, 비율이 작아져도 모서리가 뒤집히지 않는다 (BuildNineSlice가 비례 축소).
// 그리기만 바뀌므로 레이아웃은 건드리지 않고, 입력도 받지 않는다 (히트 테스트 대상 아님).
export class ProgressBar final : public Widget
{
public:
    ProgressBar(Core::Rect localRect,
        GaugeSkin skin,
        float ratio = 0.f,
        GaugeDirection direction = GaugeDirection::LeftToRight);

    void SetRatio(float ratio) { m_ratio = Clamp01(ratio); }   // 0~1로 잘라서 저장

protected:
    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override;

private:
    GaugeSkin m_skin;
    float m_ratio;
    GaugeDirection m_direction;
};