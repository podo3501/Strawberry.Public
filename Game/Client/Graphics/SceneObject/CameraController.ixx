export module Graphics.SceneObject:CameraController;

import std;
import :CameraInputState;
import :WorldCamera;

export class CameraController
{
public:
    CameraController() = default;
    ~CameraController() = default;

    void Update(const CameraInputState& input, WorldCamera& camera, float deltaTime) const noexcept
    {
        float speed = input.fastMove ? m_boostSpeed : m_moveSpeed;
        float moveAmount = speed * deltaTime;

        if (input.moveForward != 0.0f) camera.MoveForward(input.moveForward * moveAmount);
        if (input.moveRight != 0.0f) camera.MoveRight(input.moveRight * moveAmount);
        if (input.moveUp != 0.0f) camera.MoveUp(input.moveUp * moveAmount);

        float yaw = camera.GetYaw() + input.yawDelta * m_mouseSensitivity;
        float pitch = camera.GetPitch() + input.pitchDelta * m_mouseSensitivity;

        constexpr float limit = 1.55334306f; // 89 degree
        pitch = std::clamp(pitch, -limit, limit);
        camera.SetRotation(pitch, yaw);
    }

    void SetMoveSpeed(float speed) noexcept { m_moveSpeed = speed; }
    void SetBoostSpeed(float speed) noexcept { m_boostSpeed = speed; }
    void SetMouseSensitivity(float sensitivity) noexcept { m_mouseSensitivity = sensitivity; }

private:
    float m_moveSpeed{ 0.5f };
    float m_boostSpeed{ 2.0f };
    float m_mouseSensitivity{ 0.0025f };
};