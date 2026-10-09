export module Contract.Input:IKeyboardInputProvider;

import std;
import :KeyboardState;

export struct IKeyboardInputProvider
{
	virtual ~IKeyboardInputProvider() = default;
	virtual void Update() noexcept = 0;
	virtual void ProcessMessage(std::uint32_t msg, std::uint64_t wParam, std::int64_t lParam) noexcept = 0;
	virtual const KeyboardState& GetState() const noexcept = 0;
};

export std::unique_ptr<IKeyboardInputProvider> CreateDXKeyboardInputProvider();
