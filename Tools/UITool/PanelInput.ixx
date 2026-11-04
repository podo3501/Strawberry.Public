module;

#include <Imgui/imgui.h>

export module UITool.PanelInput;

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

// ImGui의 입력을 패널 입력으로 바꾼다. 프레임 사이의 마우스 상태(이전 위치, 버튼, 휠 누적)를 들고 있다.
export class PanelInputReader
{
public:
    // 패널 영역 위젯을 그린 직후, 같은 ImGui 창 안에서 부른다.
    // origin은 패널의 화면 좌표, hovered는 패널 위에 마우스가 있는지, focused는 패널 창이 포커스를 가졌는지다.
    PanelInput Read(float originX, float originY, bool hovered, bool focused)
    {
        const ImGuiIO& io = ImGui::GetIO();
        PanelInput input;

        m_mouse.prevPosition = m_mouse.position;
        m_mouse.prevWheel = m_mouse.wheel;

        if (hovered || AnyCaptured())
            m_mouse.position = { static_cast<int>(io.MousePos.x - originX), static_cast<int>(io.MousePos.y - originY) };
        else
            m_mouse.position = { kOutside, kOutside }; // 다른 창이 위를 덮고 있으면 UI가 반응하지 않게 멀리 둔다

        if (hovered)
            m_mouse.wheel += static_cast<int>(io.MouseWheel * MouseState::WHEEL_STEP);

        for (std::size_t i = 0; i < kButtons.size(); ++i)
        {
            if (hovered && io.MouseClicked[i])
                m_captured[i] = true;      // 패널 안에서 누른 버튼만 인정하고, 뗄 때까지 밖으로 나가도 이어간다
            if (!io.MouseDown[i])
                m_captured[i] = false;

            ButtonState& button = m_mouse[kButtons[i]];
            button.BeginFrame();
            button.Apply(m_captured[i]);
        }
        input.mouse = m_mouse;

        if (!focused)
            return input;

        input.shift = io.KeyShift;
        for (const KeyMapping& map : kKeys)
            if (ImGui::IsKeyPressed(map.imgui, true))
                input.keys.push_back(map.key);

        for (const ImWchar ch : io.InputQueueCharacters)
            AppendUtf8(input.text, ch);

        return input;
    }

private:
    struct KeyMapping { ImGuiKey imgui; KeyCode key; };

    static constexpr int kOutside = -100000;
    static constexpr std::array kButtons{ MouseButton::Left, MouseButton::Right, MouseButton::Middle };
    static constexpr std::array kKeys{
        KeyMapping{ ImGuiKey_Tab, KeyCode::Tab },
        KeyMapping{ ImGuiKey_Escape, KeyCode::Escape },
        KeyMapping{ ImGuiKey_Enter, KeyCode::Enter },
        KeyMapping{ ImGuiKey_KeypadEnter, KeyCode::Enter },
        KeyMapping{ ImGuiKey_Space, KeyCode::Space },
        KeyMapping{ ImGuiKey_Backspace, KeyCode::Backspace },
        KeyMapping{ ImGuiKey_Delete, KeyCode::Delete },
        KeyMapping{ ImGuiKey_Home, KeyCode::Home },
        KeyMapping{ ImGuiKey_End, KeyCode::End },
        KeyMapping{ ImGuiKey_LeftArrow, KeyCode::Left },
        KeyMapping{ ImGuiKey_RightArrow, KeyCode::Right },
        KeyMapping{ ImGuiKey_UpArrow, KeyCode::Up },
        KeyMapping{ ImGuiKey_DownArrow, KeyCode::Down },
    };

    bool AnyCaptured() const
    {
        return std::ranges::any_of(m_captured, std::identity{});
    }

    static void AppendUtf8(std::string& out, std::uint32_t codepoint)
    {
        if (codepoint < 0x20 || codepoint == 0x7F) return; // 제어 문자는 키로 따로 전달한다

        if (codepoint < 0x80)
            out.push_back(static_cast<char>(codepoint));
        else if (codepoint < 0x800)
        {
            out.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint < 0x10000)
        {
            out.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        else
        {
            out.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
    }

    MouseState m_mouse;
    std::array<bool, kButtons.size()> m_captured{};
};
