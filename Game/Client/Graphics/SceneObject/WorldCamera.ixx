export module Graphics.SceneObject:WorldCamera;

import std;
import :Camera;
import Core.Math;

constexpr float DegToRad(float deg) noexcept
{
    return deg * std::numbers::pi_v<float> / 180.0f;
}

export class WorldCamera : public Camera
{
public:
    virtual ~WorldCamera() override = default;

    WorldCamera()
    {
        SetNearFar(0.1f, 1000.f);
    }

    virtual Core::Matrix BuildProjection(
        const Core::Size& screenSize,
        const std::optional<Core::Rect>& viewport) const override
    {
        float aspect;
        if (viewport.has_value())
            aspect = viewport->width / viewport->height;
        else
            aspect = static_cast<float>(screenSize.width) / static_cast<float>(screenSize.height);

        if (aspect != m_lastAspect || GetProjVersion() != m_lastProjVersion)
        {
            m_proj = Core::CreatePerspectiveFov(DegToRad(m_fov), aspect, m_nearZ, m_farZ);
            m_lastAspect = aspect;
            m_lastProjVersion = GetProjVersion();
        }
        return m_proj;
    }

    void SetRotation(float pitch, float yaw)
    {
        m_pitch = pitch;
        m_yaw = yaw;
        m_dirty = true;
    }

    void SetFov(float fovDeg)
    {
        m_fov = fovDeg;
        MarkProjDirty();
    }

    void Move(const Core::Vector3& delta)
    {
        m_position = m_position + delta;
        m_dirty = true;
    }

    void MoveForward(float distance)
    {
        UpdateIfNeeded(); // base가 private라 직접 못 부름 -> 아래 참고
        m_position = m_position + m_forward * distance;
        m_dirty = true;
    }

    void MoveRight(float distance)
    {
        UpdateIfNeeded();
        m_position = m_position + m_right * distance;
        m_dirty = true;
    }

    void MoveUp(float distance)
    {
        m_position.y += distance;
        m_dirty = true;
    }

    Core::Vector3 GetForward() const { return m_forward; }
    Core::Vector3 GetRight() const { return m_right; }
    Core::Vector3 GetUp() const { return m_up; }

    float GetPitch() const { return m_pitch; }
    float GetYaw() const { return m_yaw; }
    float GetFov() const { return m_fov; }

protected:
    virtual void UpdateMatrices() const override
    {
        float cosPitch = std::cos(m_pitch);
        float sinPitch = std::sin(m_pitch);
        float cosYaw = std::cos(m_yaw);
        float sinYaw = std::sin(m_yaw);

        m_forward.x = sinYaw * cosPitch;
        m_forward.y = sinPitch;
        m_forward.z = cosYaw * cosPitch;
        m_forward.Normalize();

        Core::Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
        m_right = worldUp.Cross(m_forward);
        m_right.Normalize();

        m_up = m_forward.Cross(m_right);
        m_up.Normalize();

        Core::Vector3 target = m_position + m_forward;
        m_view = Core::CreateLookAt(m_position, target, worldUp);
    }

private:
    float m_pitch{ 0.0f };
    float m_yaw{ 0.0f };
    float m_fov{ 60.0f };

    mutable Core::Vector3 m_forward{ 0.0f, 0.0f, 1.0f };
    mutable Core::Vector3 m_right{ 1.0f, 0.0f, 0.0f };
    mutable Core::Vector3 m_up{ 0.0f, 1.0f, 0.0f };

    mutable Core::Matrix m_proj;
    mutable float m_lastAspect{ -1.0f };
    mutable std::uint32_t m_lastProjVersion{ 0xFFFFFFFF };
};