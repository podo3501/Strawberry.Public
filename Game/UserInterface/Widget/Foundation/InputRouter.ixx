export module UserInterface.Widget:InputRouter;

import std;
import :Widget;
import :InputEvents;
import Core.Math;

// 입력 계약(MouseState)에서 라우터가 필요한 값만 뽑은 한 프레임분 입력
export struct PointerFrame
{
    Core::Vector2 position;
    float wheelDelta{ 0.f };   // 양수 = 휠 위로 (ScrollEvent::delta와 같은 의미)
    bool leftPressed{ false };
    bool leftReleased{ false };
};

// 위젯 트리에 입력을 분배: hover 전환, 휠/누름의 부모 방향 버블링, 캡처 추적, 클릭 판정.
export class InputRouter
{
public:
    Widget* Hovered(const Widget& root) const;   // 지금 hover 중인 위젯. 없거나 파괴됐거나 트리 밖이면 nullptr
    Widget* Pressed(const Widget& root) const;   // 지금 누름을 캡처한 위젯. 없거나 파괴됐거나 트리 밖이면 nullptr
    bool Update(Widget& root, const PointerFrame& frame); // 반환값: 이 입력이 UI 위에서 일어났는가 (히트 위젯이 있거나 캡처 중)
    void Release(const Widget& root);   // hover와 누름 캡처를 풀고 위젯에 Leave/Up을 알린다 (클릭은 일어나지 않는다)

private:
    void UpdateHover(const Widget& root, Widget* hit);
    void DispatchScroll(Widget* hit, const ScrollEvent& e);
    void DispatchPress(Widget* hit, const PointerEvent& e);
    void DispatchMove(const Widget& root, const PointerEvent& e);
    void DispatchRelease(const Widget& root, Widget* hit, const PointerEvent& e);

    std::weak_ptr<Widget*> m_hovered;
    std::weak_ptr<Widget*> m_pressed;
};