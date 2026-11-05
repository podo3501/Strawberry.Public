export module Contract.Input.Service:ButtonState;

export struct ButtonState
{
    bool previous{ false };
    bool current{ false };
    bool pressedThisFrame{ false };
    bool releasedThisFrame{ false };

    void BeginFrame() noexcept
    {
        previous = current;
        pressedThisFrame = false;
        releasedThisFrame = false;
    }

    void Apply(bool isDown) noexcept
    {
        if (isDown && !current) { current = true;  pressedThisFrame = true; }
        if (!isDown && current) { current = false; releasedThisFrame = true; }
    }

    [[nodiscard]] bool IsPressed() const noexcept { return pressedThisFrame; }
    [[nodiscard]] bool IsReleased() const noexcept { return releasedThisFrame; }
    [[nodiscard]] bool IsHeld() const noexcept { return previous && current; }
	[[nodiscard]] bool IsDown() const noexcept { return current; }   // Pressed와 Held를 포함한 상태
    [[nodiscard]] bool IsUp() const noexcept { return !current && !releasedThisFrame; }
};