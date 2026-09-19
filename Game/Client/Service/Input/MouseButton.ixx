export module Client.Input:MouseButton;

import std;

namespace Client
{
    export enum class MouseButton : std::uint8_t
    {
        Left,
        Right,
        Middle,
        X1,
        X2,

        Count
    };
}