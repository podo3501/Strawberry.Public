export module Service.Input:KeyRepeater;

import std;
import Contract.Input.Shared;
import Contract.Input.Service;

// 키를 계속 누르고 있을 때 OS의 자동 반복처럼 반복 시점을 알려 준다 (InputService 안에서만 쓴다).
// 입력 백엔드는 키를 누른 프레임에 한 번만 알리므로, 누른 지 kKeyRepeatDelay초 뒤부터 kKeyRepeatInterval초마다를 여기서 센다.
export class KeyRepeater
{
public:
    // 프레임마다 키마다 한 번 부른다. 이 프레임이 반복 시점이면 true. 누른 프레임은 반복이 아니라 false다.
    // 눌린 프레임을 보지 못한 채 눌려 있기만 한 키는 반복하지 않는다
    bool Poll(KeyCode key, const ButtonState& state, float dt) noexcept
    {
        Hold& hold = m_holds[static_cast<std::size_t>(key)];

        if (state.IsPressed())
        {
            hold = { true, kKeyRepeatDelay };
            return false;
        }

        if (!state.IsDown())
        {
            hold.active = false;
            return false;
        }

        if (!hold.active)
            return false;

        hold.untilNext -= dt;
        if (hold.untilNext > 0.f)
            return false;

        hold.untilNext = kKeyRepeatInterval;   // 프레임이 길게 끊겨도 한 프레임에 한 번만 발생한다
        return true;
    }

private:
    struct Hold
    {
        bool active{ false };
        float untilNext{ 0.f };
    };

    std::array<Hold, static_cast<std::size_t>(KeyCode::Count)> m_holds{};
};
