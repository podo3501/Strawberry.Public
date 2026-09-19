export module Client.Input:IKeyboardInputProvider;

import std;
import :KeyboardState;

namespace Client
{
	export struct IKeyboardInputProvider
	{
		virtual ~IKeyboardInputProvider() = default;
		virtual void Update() noexcept = 0;
		virtual const KeyboardState& GetState() const noexcept = 0;
	};

	export std::unique_ptr<IKeyboardInputProvider> CreateDXKeyboardInputProvider();
}
