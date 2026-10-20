export module Contract.Input.Backend:IInputBackend;

import std;
import :InputFrame;

export struct IInputBackend
{
	virtual ~IInputBackend() = default;
	[[nodiscard]] virtual InputFrame TakeFrame() noexcept = 0;
};

export using NativeWindowHandle = void*;
export std::unique_ptr<IInputBackend> CreateDxInputBackend(NativeWindowHandle handle);