export module Client.Input.Contract:IMouseInputProvider;

import std;
import :MouseState;

namespace Client
{
    export struct IMouseInputProvider
    {
        virtual ~IMouseInputProvider() = default;
        virtual void Update() noexcept = 0;
        virtual const MouseState& GetState() const noexcept = 0;
    };

    export using NativeWindowHandle = void*;
    export std::unique_ptr<IMouseInputProvider> CreateDXMouseInputProvider(NativeWindowHandle handle);
}
