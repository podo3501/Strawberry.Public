module Service.InputService;

import std;

InputService::~InputService() = default;

InputService::InputService(std::unique_ptr<IInputBackend> backend) noexcept
    : m_backend{ std::move(backend) }
{}

std::unique_ptr<InputService> InputService::Create(std::unique_ptr<IInputBackend> backend)
{
    if (!backend) return nullptr;

    return std::unique_ptr<InputService>(new InputService(std::move(backend)));
}

bool InputService::AreKeysCombo(std::initializer_list<KeyCode> heldKeys, KeyCode pressedKey) const noexcept
{
    return AreKeysDown(heldKeys) && IsKeyPressed(pressedKey);
}

bool InputService::AreKeysCombo(std::initializer_list<KeyCode> heldKeys, MouseButton pressedButton) const noexcept
{
    return AreKeysDown(heldKeys) && IsMouseButtonPressed(pressedButton);
}

void InputService::Update() noexcept
{
    const InputFrame frame = m_backend->TakeFrame();

    for (auto& k : m_keyboardState.keys)
        k.BeginFrame();                       // 이벤트 없는 키도 previous/플래그 갱신

    for (const KeyEvent& e : frame.keyEvents)
        m_keyboardState[e.key].Apply(e.down); // 순서대로 적용

    for (auto& b : m_mouseState.buttons)
        b.BeginFrame();

    for (const MouseButtonEvent& e : frame.mouseButtonEvents)
        m_mouseState[e.button].Apply(e.down);

    m_mouseState.prevPosition = m_mouseState.position;
    m_mouseState.position = frame.position;

    if (!m_mouseInitialized)    // 첫 프레임은 이전 위치가 없으므로 delta가 튀지 않게 맞춰 줌
    {
        m_mouseState.prevPosition = m_mouseState.position;
        m_mouseInitialized = true;
    }

    m_mouseState.prevWheel = m_mouseState.wheel;
    m_mouseState.wheel += frame.wheelDelta;
}

bool InputService::AreKeysDown(std::initializer_list<KeyCode> keys) const noexcept
{
    if (keys.size() == 0)
        return false;

    for (KeyCode key : keys)
    {
        if (!m_keyboardState[key].IsDown())
            return false;
    }

    return true;
}