export module Client.Input:IMouseInputProvider;

import std;
import :MouseState;

struct HWND__;
using HWND = struct HWND__*;

namespace Client
{
    export struct IMouseInputProvider
    {
        virtual ~IMouseInputProvider() = default;
        virtual void Update() noexcept = 0;
        virtual const MouseState& GetState() const noexcept = 0;
    };

    export std::unique_ptr<IMouseInputProvider> CreateDXMouseInputProvider(HWND hwnd);
}
