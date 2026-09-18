export module InputState;

export struct InputState
{
    bool previous{ false };
    bool current{ false };

    void Update(bool isDown)
    {
        previous = current;
        current = isDown;
    }

    [[nodiscard]] bool IsPressed() const noexcept { return current && !previous; }
    [[nodiscard]] bool IsReleased() const noexcept { return !current && previous; }
    [[nodiscard]] bool IsHeld() const noexcept { return current; }
    [[nodiscard]] bool IsUp() const noexcept { return !current; }
};