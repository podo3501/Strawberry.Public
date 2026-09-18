export module IKeyboardInputProvider;

import std;
import KeyboardState;

export struct IKeyboardInputProvider
{
	virtual ~IKeyboardInputProvider() = default;
	virtual void Update() noexcept = 0;
	virtual const KeyboardState& GetState() const noexcept = 0;
};

export std::unique_ptr<IKeyboardInputProvider> CreateDXKeyboardInputProvider();
