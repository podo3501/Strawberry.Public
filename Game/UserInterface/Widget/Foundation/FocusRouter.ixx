export module UserInterface.Widget:FocusRouter;

import std;
import :Widget;
import :InputEvents;

// 키보드 포커스를 관리한다: Tab 이동, 마우스 누름에 따른 포커스 이동, 키 입력의 버블링.
// 모달(TrapsFocus인 위젯)이 있으면 그 서브트리 안에서만 이동하고 키를 받는다. 여러 개면 맨 위(나중에 붙은) 것이 기준이다.
export class FocusRouter
{
public:
    // 누른 프레임에 부른다. pressed: 이번 누름을 캡처한 위젯 (없으면 nullptr).
    // 포커스를 받는 가장 가까운 자기/조상 위젯으로 옮기고, 없으면 포커스를 푼다.
    void OnPress(const Widget& root, Widget* pressed);

    // Tab/Shift+Tab은 포커스 이동에 예약된 키다 (위젯이 가로챌 수 없다).
    // 그 밖의 키는 포커스 위젯(없거나 모달 밖이면 모달 자체)에서 부모 방향으로 버블된다.
    // 반환: 소비되었는가 (Tab은 이동할 위젯이 있으면 true)
    bool OnKeyDown(Widget& root, const KeyEvent& e);

    // 입력된 문자를 포커스 위젯에게만 전달한다 (버블링 없음). 포커스가 없거나 모달 밖이면 버린다.
    // 반환: 포커스 위젯에게 전달되었는가
    bool OnText(Widget& root, std::string_view utf8);

    // 시간 진행(초)을 포커스 위젯에게만 전달한다.
    void Tick(Widget& root, float dt);

    void Release(const Widget& root);   // 포커스를 풀고 포커스 위젯에 Blur를 알린다

private:
    static Widget* FindScope(Widget& w); // 맨 위의 TrapsFocus 위젯 (없으면 nullptr)
    static void Collect(Widget& w, std::vector<Widget*>& out); // 트리 순서대로 포커스 가능한 위젯 수집

    bool MoveFocus(const Widget& root, Widget& scope, int step);  // step: +1 다음, -1 이전
    void SetFocus(const Widget& root, Widget* target, bool viaKeyboard);

    std::weak_ptr<Widget*> m_focused;
};