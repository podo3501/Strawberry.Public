export module UIVisualizer.WidgetNudger;

import std;
import UserInterface;
import Contract.Input.Service;
import Core.Math;

// 고른 위젯을 키보드로 조금씩 옮기고, 옮긴 것을 JSON에 쓸 때를 알려 준다 (구현: WidgetNudger.cpp)
// 옮기는 동안에는 메모리의 위젯 오프셋만 바꿔 화면에 바로 보여 주고, 화살표 키를 모두 뗀 뒤 kSettleTime이 지나면 Tick이 true를 돌려준다.
// 그때 호출하는 쪽이 JSON에 한 번만 써서, 길게 옮긴 것이 이력의 한 단계가 되게 한다.
export class WidgetNudger
{
public:
    static constexpr float kStep = 1.f;          // 화살표 한 번에 옮기는 픽셀
    static constexpr float kBigStep = 8.f;       // Shift를 누른 채 옮기는 픽셀
    static constexpr float kSettleTime = 0.5f;   // 화살표 키를 모두 뗀 뒤 이만큼 지나면 쓸 때가 된 것이다(초)

    // 프레임마다 부른다. 화살표(Ctrl과 함께 누른 것은 제외)를 눌렀으면 selected를 옮기고(Shift는 kBigStep), 키가 눌려 있는지에 따라 Tick을 한다.
    // 쓸 때가 되었으면 true (호출하는 쪽이 JSON에 쓰고 Reset한다)
    bool Update(float dt, const IInputService& input, std::span<Widget* const> selected);

    // 위젯들의 앵커 오프셋을 delta만큼 옮긴다. 컨테이너(Stack 등)가 배치해서 오프셋으로 움직이지 않는 위젯은 원래대로 두고 건너뛴다.
    // 하나라도 옮겼으면 true이고 쓸 것이 생겼다고 기억한다 (기다린 시간은 다시 0부터)
    bool Move(std::span<Widget* const> widgets, const Core::Vector2& delta);

    // 프레임마다 부른다. 옮긴 것이 있고 keyHeld(화살표가 눌려 있다)가 아니면 시간을 세고, kSettleTime이 지나면 true(쓸 때가 되었다)를 돌려준다.
    // 쓴 뒤에는 Reset을 불러야 한다 (부르지 않으면 계속 true)
    bool Tick(float dt, bool keyHeld);

    bool HasPending() const noexcept { return m_pending; }

    // 쓸 것이 있다는 기억을 버린다 (위젯이 새로 만들어졌거나 쓰는 중일 때)
    void Reset() noexcept { m_pending = false; m_idle = 0.f; }

private:
    bool m_pending{ false };
    float m_idle{ 0.f };   // 마지막으로 옮기거나 키가 눌려 있던 뒤로 흐른 시간
};
