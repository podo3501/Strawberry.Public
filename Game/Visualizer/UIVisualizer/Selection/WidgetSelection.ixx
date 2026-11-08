export module UIVisualizer.WidgetSelection;

import std;
import UserInterface;
import Core.Math;

// 화면에서 클릭해 고른 위젯의 목록 (구현: WidgetSelection.cpp)
// 고른 순서가 곧 번호다(순서 + 1). 위젯을 가리키는 포인터만 들고 있으므로, 레이아웃을 다시 읽어 위젯이 새로 만들어지면 Clear로 비워야 한다.
export class WidgetSelection
{
public:
    // layerRoot 아래에서 point(화면 좌표)에 보이는 가장 위의 위젯을 찾아 목록에 넣는다.
    // 위젯은 깊이 들어간 쪽(자식)과 나중에 그려지는 쪽(나중에 붙인 형제)이 위다. 입력에 반응하지 않는 위젯(이미지, 글자 등)도 고른다.
    // 이미 목록에 있는 위젯이면 목록에서 뺀다. 레이어 루트 자신은 고르지 않는다.
    // 위젯이 없는 곳이면 아무것도 바꾸지 않고 false
    bool Toggle(const Widget& layerRoot, const Core::Vector2& point);

    void Clear() { m_items.clear(); }

    // 고른 순서대로. 번호는 인덱스 + 1
    std::span<const Widget* const> Items() const { return m_items; }

private:
    std::vector<const Widget*> m_items;
};
