export module UserInterface:UserInterface;

import std;
import UserInterface.Widget;
import Contract.Render.UI;
import Contract.Render.ITextMeasurer;
import Contract.Input.Service;
import Core.Math;

// 아래에서 위로 그려지고, 위쪽이 입력을 먼저 받는다. 같은 레이어 안에서는 나중에 붙인 위젯이 위.
export enum class UILayer { Hud, Window, Popup, Tooltip };

export class UserInterface
{
public:
    explicit UserInterface(ITextMeasurer& measurer);   // 텍스트를 쓰지 않는 UI는 없으므로 처음에 한 번 받는다. UserInterface보다 오래 살아야 한다

    template<std::derived_from<Widget> T>
    T& Attach(std::unique_ptr<T> widget, UILayer layer = UILayer::Hud)
    {
        return LayerRoot(layer).Attach(std::move(widget));
    }

    template<std::derived_from<Widget> T>
    T& Attach(std::unique_ptr<T> widget, UILayer layer, std::size_t index)
    {
        return LayerRoot(layer).Attach(std::move(widget), index);
    }

    ITextMeasurer& TextMeasurer() const { return m_measurer; }   // Text 계열 위젯을 만들 때 넘긴다. 저장된 UI를 불러올 때도 이 측정기를 쓴다

    // 레이어의 루트. Dropdown처럼 다른 레이어에 팝업을 띄우는 위젯에게 넘긴다. 화면 전체를 덮고 원점이 화면 (0,0)이다
    Widget& Layer(UILayer layer) { return LayerRoot(layer); }

    void Draw(IOverlayView& overlay) const;
    bool Update(const MouseState& mouse); // 입력 처리. 프레임마다 Tick보다 먼저 부른다
    bool OnKeyDown(KeyCode key, bool shift = false); // 키를 누른 프레임에 한 번 부른다 (Tab, Escape, Enter, Space 등 UI가 쓰는 키만 넘기면 된다). 소비되면 true: 게임이 같은 키를 또 처리하지 않도록 건너뛴다.
    bool OnTextInput(std::string_view utf8); // 이번 프레임에 입력된 문자(UTF-8)를 포커스 위젯에 넘긴다. 전달되었으면 true
    void Tick(float dt); // 시간 진행(초). 입력이 없어도 부른다 (툴팁 지연 등). 일시정지 중에도 UI를 돌리려면 실시간 dt를 넘긴다
    void Resize(float width, float height);   // 화면 크기 변경. root를 (0,0,w,h)로 두고 전체 재배치

    // 툴팁을 만드는 방법과 타이밍. 호출하지 않으면 툴팁은 뜨지 않는다
    void ConfigureTooltip(TooltipFactory factory, TooltipSettings settings = {});

private:
    static constexpr std::size_t kLayerCount = static_cast<std::size_t>(UILayer::Tooltip) + 1;

    Group& LayerRoot(UILayer layer) { return *m_layers[static_cast<std::size_t>(layer)]; }

    ITextMeasurer& m_measurer;
    Group m_root;
    std::array<Group*, kLayerCount> m_layers{};   // m_root의 자식. 생성자에서 채운다
    InputRouter m_input;
    FocusRouter m_focus;
    TooltipController m_tooltip;
    Core::Vector2 m_pointer{};                    // 마지막 Update의 포인터 위치 (툴팁 위치용)
};