export module Contract.Input:IMouseInputProvider;

import std;
import :MouseState;

export struct IMouseInputProvider
{
    virtual ~IMouseInputProvider() = default;
    virtual void Update() noexcept = 0;
    virtual void ProcessMessage(std::uint32_t msg, std::uint64_t wParam, std::int64_t lParam) noexcept = 0;
    virtual const MouseState& GetState() const noexcept = 0;
};

export using NativeWindowHandle = void*;
export std::unique_ptr<IMouseInputProvider> CreateDXMouseInputProvider(NativeWindowHandle handle);
