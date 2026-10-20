export module Contract.Input.Backend:InputFrame;

import std;
import Contract.Input.Shared;
import Core.Math;

export struct KeyEvent
{
    KeyCode key{};
    bool down{ false };   // true: 눌림, false: 뗌
};

export struct MouseButtonEvent
{
	MouseButton button{};
	bool down{ false };   // true: 눌림, false: 뗌
};

export struct InputFrame
{
    std::vector<KeyEvent> keyEvents;
    std::vector<MouseButtonEvent> mouseButtonEvents;

    Core::Point position{}; // Take 시점의 현재 위치 (backend가 마지막 값을 계속 유지)
    int wheelDelta{ 0 }; // 이번 프레임에 누적된 휠 변화량 (한 틱 = 120)

    [[nodiscard]] bool Empty() const noexcept
    {
        return keyEvents.empty() && mouseButtonEvents.empty() && wheelDelta == 0;
    }
};