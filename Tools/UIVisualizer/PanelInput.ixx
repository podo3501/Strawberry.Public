export module UIVisualizer.PanelInput;

import std;
import Contract.Input.Service;
import Contract.Input.Shared;
import Core.Math;

// 미리보기 패널에서 한 프레임 동안 모은 입력. 엔진의 UserInterface가 받는 형태 그대로다.
export struct PanelInput
{
    MouseState mouse;           // 위치는 패널 왼쪽 위가 원점인 좌표
    std::vector<KeyCode> keys;  // 이번 프레임에 눌린 키 (자동 반복 포함)
    bool shift{ false };
    std::string text;           // 이번 프레임에 입력된 문자 (UTF-8)
};

// ImGui의 입력을 패널 입력으로 바꾼다. 프레임 사이의 마우스 상태(이전 위치, 버튼, 휠 누적)를 들고 있다. (구현: PanelInput.cpp)
export class PanelInputReader
{
public:
    // 패널 영역 위젯을 그린 직후, 같은 ImGui 창 안에서 부른다.
    // origin은 패널의 화면 좌표, hovered는 패널 위에 마우스가 있는지, focused는 패널 창이 포커스를 가졌는지다.
    PanelInput Read(float originX, float originY, bool hovered, bool focused);

private:
    static constexpr std::array kButtons{ MouseButton::Left, MouseButton::Right, MouseButton::Middle };

    bool AnyCaptured() const { return std::ranges::any_of(m_captured, std::identity{}); }

    MouseState m_mouse;
    std::array<bool, kButtons.size()> m_captured{};
};
