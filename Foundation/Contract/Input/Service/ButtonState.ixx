export module Contract.Input.Service:ButtonState;

// 키를 계속 누르고 있을 때 IsRepeated가 참이 되는 시점. 누른 지 kKeyRepeatDelay초 뒤부터 kKeyRepeatInterval초마다 (OS의 자동 반복과 비슷하다)
export inline constexpr float kKeyRepeatDelay = 0.5f;
export inline constexpr float kKeyRepeatInterval = 0.033f;

export struct ButtonState
{
    bool previous{ false };
    bool current{ false };
    bool pressedThisFrame{ false };
    bool releasedThisFrame{ false };
    bool repeatedThisFrame{ false };   // 키보드 키만 입력 서비스가 채운다

    void BeginFrame() noexcept
    {
        previous = current;
        pressedThisFrame = false;
        releasedThisFrame = false;
        repeatedThisFrame = false;
    }

    void Apply(bool isDown) noexcept
    {
        if (isDown && !current) { current = true;  pressedThisFrame = true; }
        if (!isDown && current) { current = false; releasedThisFrame = true; }
    }

    [[nodiscard]] bool IsPressed() const noexcept { return pressedThisFrame; }
    [[nodiscard]] bool IsReleased() const noexcept { return releasedThisFrame; }
    [[nodiscard]] bool IsRepeated() const noexcept { return repeatedThisFrame; }   // 누르고 있어서 반복이 일어나는 프레임. 누른 프레임은 IsPressed이고 여기에는 들지 않는다 (글자 지우기처럼 반복이 필요한 곳은 IsPressed() || IsRepeated())
    [[nodiscard]] bool IsHeld() const noexcept { return previous && current; }
	[[nodiscard]] bool IsDown() const noexcept { return current; }   // Pressed와 Held를 포함한 상태
    [[nodiscard]] bool IsUp() const noexcept { return !current && !releasedThisFrame; }
};