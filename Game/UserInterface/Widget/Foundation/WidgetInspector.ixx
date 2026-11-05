export module UserInterface.Widget:WidgetInspector;

import std;
import :Widget;
import Core.Math;

// 위젯 트리를 읽기 전용으로 들여다보는 통로. 툴이 트리 패널과 선택 표시를 만들 때 쓴다.
// 구조를 바꾸거나 위젯 내부에 의존하게 하지 않도록, 읽는 것만 Widget이 이 클래스에게 열어 준다.
export class WidgetInspector
{
public:
    static std::span<const std::unique_ptr<Widget>> Children(const Widget& widget) { return widget.m_children; }

    // 화면 좌표의 rect (부모들의 위치가 더해진 값)
    static Core::Rect ScreenRect(const Widget& widget) { return widget.ScreenRect(); }

    // 레이아웃 속성 (값을 바꾸는 것은 Widget의 공개 Set 함수를 쓴다)
    static const Anchor& GetAnchor(const Widget& widget) { return widget.m_anchor; }
    static SizePolicy WidthPolicy(const Widget& widget) { return widget.m_widthPolicy; }
    static SizePolicy HeightPolicy(const Widget& widget) { return widget.m_heightPolicy; }
    static const std::string& Tooltip(const Widget& widget) { return widget.m_tooltip; }

    // 위젯의 클래스 이름 (네임스페이스 포함, class/struct 접두어와 모듈 표기 제외)
    static std::string TypeName(const Widget& widget)
    {
        std::string name = typeid(widget).name();   // MSVC는 모듈 안의 클래스에 "class Button[UserInterface.Widget]" 형태로 준다
        if (const auto module = name.find('['); module != std::string::npos)
            name.erase(module);
        if (const auto space = name.rfind(' '); space != std::string::npos)
            name.erase(0, space + 1);
        return name;
    }
};
