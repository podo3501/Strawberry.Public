export module Game.UserInterface.Widget:InputEvents;

import Core.Math;
import Contract.Input.Shared;

export struct PointerEvent
{
    Core::Vector2 screenPos;
};

export struct ScrollEvent
{
    Core::Vector2 screenPos;
    float delta;   // 양수 = 휠을 위로 굴림 (내용은 아래로 내려와 보인다)
};

export struct KeyEvent
{
    KeyCode key;
    bool shift;    // 누른 순간 Shift가 눌려 있었는가 (Shift+Tab)
};