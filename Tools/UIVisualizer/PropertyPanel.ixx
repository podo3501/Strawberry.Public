export module UIVisualizer.PropertyPanel;

import std;
import UserInterface;
import Core.Math;

// 선택한 위젯의 공통 레이아웃 속성(앵커, 피벗, 오프셋, 크기, 크기 정책, 툴팁)을 보여 주고 고치는 패널.
// 값을 바꿀 때는 위젯의 Set 함수만 쓰고, 읽을 때는 WidgetInspector를 쓴다. (구현: PropertyPanel.cpp)
export class PropertyPanel
{
public:
    void Draw(Widget* selected);

private:
    void DrawLayout(Widget& widget);
    void DrawTooltip(Widget& widget);

    std::array<char, 256> m_tooltipBuffer{};
    const Widget* m_tooltipTarget{ nullptr };
    bool m_tooltipEditing{ false };
};
