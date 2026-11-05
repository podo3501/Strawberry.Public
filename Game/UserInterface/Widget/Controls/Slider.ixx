export module UserInterface.Widget:Slider;

import std;
import :Widget;
import :InputEvents;
import :Gauge;
import :WidgetResources;
import Contract.Render.UI;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

export struct SliderSkin
{
    GaugeSkin bar; // 트랙(배경)과 채움
    BrushHandle thumb{};
    Core::Vector2 thumbSize{};
};

export enum class SliderAxis { Horizontal, Vertical, Count };   // Count는 저장용 이름 표(Slider.cpp)가 쓴다

// 슬라이더. 값은 0~1 비율이다. 가로는 왼쪽이 0, 세로는 아래쪽이 0이다.
// 누르면 그 자리로 값이 점프하고, 누른 채 움직이면 포인터가 위젯 밖으로 나가도 계속 따라간다 (포인터 캡처).
// SetValue는 콜백을 부르지 않고, 사용자 조작으로 값이 실제로 바뀔 때만 SetOnChanged 콜백이 불린다.
export class Slider final : public Core::TypeNode<Slider, Widget>
{
public:
    Slider(Core::Rect localRect, 
        SliderSkin skin, 
        SliderAxis axis = SliderAxis::Horizontal, 
        float value = 0.f);

    void SetValue(float value) { m_value = Clamp01(value); }
    float Value() const { return m_value; }
    void SetOnChanged(std::function<void(float)> onChanged) { m_onChanged = std::move(onChanged); }

protected:
    bool Serialize(Serializer& serializer, IWidgetResources& resources) override;   // 구현: Slider.cpp
    bool IsHitTestable() const override { return true; }
    bool OnPointerDown(const PointerEvent& e) override;   // true를 반환해 캡처한다
    void OnPointerMove(const PointerEvent& e) override;
    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override;

private:
    bool IsHorizontal() const { return m_axis == SliderAxis::Horizontal; }
    float Travel() const   // 썸이 움직이는 거리 (주축 기준)
    {
        return (std::max)(0.f, IsHorizontal() ? Size().x - m_skin.thumbSize.x : Size().y - m_skin.thumbSize.y);
    }
    void SetFromPointer(const Core::Vector2& screenPos);

    SliderSkin m_skin;
    SliderAxis m_axis;
    float m_value;
    std::function<void(float)> m_onChanged;
};