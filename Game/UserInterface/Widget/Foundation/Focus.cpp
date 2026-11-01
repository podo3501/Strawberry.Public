module Game.UserInterface.Widget;

import std;
import :Focus;
import :Widget;
import :InputEvents;
import Contract.Input.Shared;

void FocusRouter::OnPress(const Widget& root, Widget* pressed)
{
    Widget* target = pressed;
    while (target && !target->IsFocusable())
        target = target->Parent();

    SetFocus(root, target, false);
}

bool FocusRouter::OnKeyDown(Widget& root, const KeyEvent& e)
{
    Widget* trap = FindScope(root);
    Widget& scope = trap ? *trap : root;

    if (e.key == KeyCode::Tab)
        return MoveFocus(root, scope, e.shift ? -1 : 1);

    // 포커스가 없거나 모달 밖에 있으면 모달 자체에서 시작한다 (모달이 없으면 nullptr이라 아무도 받지 않는다)
    Widget* focused = Widget::Resolve(root, m_focused);
    Widget* start = (focused && focused->IsDescendantOf(scope)) ? focused : trap;

    for (Widget* w = start; w; w = w->Parent())
        if (w->OnKeyDown(e))
            return true;   // 콜백이 위젯을 파괴할 수 있으니 소비된 뒤에는 w를 건드리지 않는다
    return false;
}

// 트리 순서(앞이 아래, 뒤가 위)에서 마지막으로 만나는 모달 = 가장 위에 떠 있는 모달
Widget* FocusRouter::FindScope(Widget& w)
{
    Widget* found = w.TrapsFocus() ? &w : nullptr;
    for (const auto& child : w.Children())
        if (Widget* inner = FindScope(*child))
            found = inner;
    return found;
}

void FocusRouter::Collect(Widget& w, std::vector<Widget*>& out)
{
    if (w.IsFocusable())
        out.push_back(&w);
    for (const auto& child : w.Children())
        Collect(*child, out);
}

bool FocusRouter::MoveFocus(const Widget& root, Widget& scope, int step)
{
    std::vector<Widget*> order;
    Collect(scope, order);
    if (order.empty())
        return false;

    const auto count = static_cast<std::ptrdiff_t>(order.size());
    const auto current = std::find(order.begin(), order.end(), Widget::Resolve(root, m_focused));

    std::ptrdiff_t next = step > 0 ? 0 : count - 1;   // 포커스가 없거나 범위 밖이면 처음(Shift는 마지막)부터
    if (current != order.end())
        next = (std::distance(order.begin(), current) + step + count) % count;

    SetFocus(root, order[static_cast<std::size_t>(next)], true);
    return true;
}

void FocusRouter::SetFocus(const Widget& root, Widget* target, bool viaKeyboard)
{
    Widget* current = Widget::Resolve(root, m_focused);
    if (current == target)
    {
        if (target && viaKeyboard)
            target->OnFocus(true);   // 마우스로 받은 포커스에서 Tab을 누르면 표시를 켠다
        return;
    }

    if (current)
        current->OnBlur();
    if (target)
        target->OnFocus(viaKeyboard);

    m_focused = target ? target->Handle() : std::weak_ptr<Widget*>{};
}