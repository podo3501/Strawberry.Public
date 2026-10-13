module;

#include "DirectXTK12/Mouse.h"

export module DxInputBackend:MouseButtonMap;

import std;
import Contract.Input.Types;

using DXMouseState = DirectX::Mouse::State;
export constexpr std::pair<MouseButton, bool DXMouseState::*> MouseButtonMap[] =
{
    { MouseButton::Left, &DXMouseState::leftButton },
    { MouseButton::Right, &DXMouseState::rightButton },
    { MouseButton::Middle, &DXMouseState::middleButton },
    { MouseButton::X1, &DXMouseState::xButton1 },
    { MouseButton::X2, &DXMouseState::xButton2 },
};
