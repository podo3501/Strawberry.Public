module Game.UserInterface.Widget;

import std;
import :InputRouter;

bool InputRouter::Update(Widget& root, const PointerFrame& frame)
{
    const PointerEvent e{ frame.position };
    Widget* hit = root.HitTest(frame.position);

    UpdateHover(root, hit);

    if (frame.wheelDelta != 0.f)
        DispatchScroll(hit, ScrollEvent{ frame.position, frame.wheelDelta });

    if (frame.leftPressed)
        DispatchPress(hit, e);
    else
        DispatchMove(root, e);   // 누른 프레임은 OnPointerDown이 이미 위치를 받았다

    if (frame.leftReleased)
        DispatchRelease(root, hit, e);

    return hit != nullptr || Resolve(root, m_pressed) != nullptr;
}

Widget* InputRouter::Resolve(const Widget& root, const std::weak_ptr<Widget*>& handle)
{
    const auto token = handle.lock();
    Widget* w = token ? *token : nullptr;
    return (w && w->IsDescendantOf(root)) ? w : nullptr;
}

void InputRouter::UpdateHover(const Widget& root, Widget* hit)
{
    // 캡처 중에는 캡처한 위젯(과 자손) 위에서만 hover를 갱신한다.
    // 드래그하며 지나가는 다른 위젯이 반응하지 않게 하고, 캡처한 위젯은 밖으로 나가면 Leave를 받는다.
    if (Widget* captured = Resolve(root, m_pressed))
        if (!hit || !hit->IsDescendantOf(*captured))
            hit = nullptr;

    Widget* hovered = Resolve(root, m_hovered);
    if (hit == hovered)
        return;

    if (hovered)
        hovered->OnPointerLeave();

    if (hit)
        hit->OnPointerEnter();

    m_hovered = hit ? hit->Handle() : std::weak_ptr<Widget*>{};
}

void InputRouter::DispatchScroll(Widget* hit, const ScrollEvent& e)
{
    for (Widget* w = hit; w; w = w->Parent())
        if (w->OnScroll(e))
            break;
}

// 누름: hit에서 부모 쪽으로 올라가며 처음 true를 반환하는 위젯이 캡처
void InputRouter::DispatchPress(Widget* hit, const PointerEvent& e)
{
    Widget* consumer = nullptr;
    for (Widget* w = hit; w && !consumer; w = w->Parent())
        if (w->OnPointerDown(e))
            consumer = w;

    m_pressed = consumer ? consumer->Handle() : std::weak_ptr<Widget*>{};
}

// 캡처한 위젯에만 전달한다. 포인터가 위젯 밖으로 나가도 계속 받는다
void InputRouter::DispatchMove(const Widget& root, const PointerEvent& e)
{
    if (Widget* pressed = Resolve(root, m_pressed))
        pressed->OnPointerMove(e);
}

// 뗌: 캡처한 위젯에 Up 전달, 뗀 위치가 그 위젯(또는 자손) 위면 Click
void InputRouter::DispatchRelease(const Widget& root, Widget* hit, const PointerEvent& e)
{
    if (Widget* pressed = Resolve(root, m_pressed))
    {
        const bool inside = hit && hit->IsDescendantOf(*pressed);   // 콜백이 위젯을 파괴할 수 있으니 먼저 계산
        pressed->OnPointerUp(e);
        if (inside)
            pressed->OnClick(e);
    }
    m_pressed.reset();
}