export module Game.UserInterface.Widget:InputEvents;

import Core.Math;

export struct PointerEvent
{
    Core::Vector2 screenPos;
};

export struct ScrollEvent
{
    Core::Vector2 screenPos;
    float delta;   // 양수 = 휠을 위로 굴림 (내용은 아래로 내려와 보인다)
};