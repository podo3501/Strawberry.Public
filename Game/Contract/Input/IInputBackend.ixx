export module Contract.Input.Interfaces:IInputBackend;

import std;
import Contract.Input.Types;

export struct IInputBackend
{
	virtual ~IInputBackend() = default;
	[[nodiscard]] virtual InputFrame TakeFrame() noexcept = 0;
};

export using NativeWindowHandle = void*;
export std::unique_ptr<IInputBackend> CreateDxInputBackend(NativeWindowHandle handle);