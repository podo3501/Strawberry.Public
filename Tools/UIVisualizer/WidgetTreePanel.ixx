export module UIVisualizer.WidgetTreePanel;

import std;
import UserInterface;
import Core.Math;

// UserInterface의 위젯 트리를 보여 주고, 클릭한 위젯을 선택하는 패널. (구현: WidgetTreePanel.cpp)
export class WidgetTreePanel
{
public:
    // "Widgets" 창을 그린다. 선택은 이 안에서 갱신된다.
    void Draw(UserInterface& ui);

    // 선택한 위젯의 테두리를 현재 ImGui 창(미리보기 패널)에 그린다. origin은 패널의 화면 좌표다.
    void DrawSelectionOutline(float originX, float originY) const;

    Widget* Selected() const { return m_selected; }

private:
    void DrawNode(Widget& widget, std::string_view layerName = {});

    Widget* m_selected{ nullptr };
};
