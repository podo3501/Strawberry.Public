export module Graphics.SceneObject:Camera;

import std;
import Core.Math;
import Client.Render.View;

export class Camera
{
public:
    virtual ~Camera() = default;

    virtual Core::Matrix BuildProjection(
        const Core::Size& screenSize, 
        const std::optional<Core::Rect>& viewport) const = 0;

    void SetPosition(const Core::Vector3& pos)
    {
        m_position = pos;
        m_dirty = true;
    }

    const Core::Vector3& GetPosition() const { return m_position; }

    void SetNearFar(float nearZ, float farZ)
    {
        m_nearZ = nearZ;
        m_farZ = farZ;
        ++m_projVersion;
    }

    float GetNearZ() const { return m_nearZ; }
    float GetFarZ() const { return m_farZ; }

    std::uint32_t GetProjVersion() const { return m_projVersion; }

    const Core::Matrix& GetView() const
    {
        UpdateIfNeeded();
        return m_view;
    }

    CameraData BuildData(
        const Core::Size& screenSize, 
        const std::optional<Core::Rect>& viewport) const
    {
        CameraData data;
        data.view = GetView();
        data.position = GetPosition();
        data.proj = BuildProjection(screenSize, viewport);

        return data;
    }

protected:
    virtual void UpdateMatrices() const = 0;

    void UpdateIfNeeded() const
    {
        if (!m_dirty) return;

        UpdateMatrices();
        m_dirty = false;
    }

    void MarkProjDirty() { ++m_projVersion; }

    Core::Vector3 m_position{ 0.f, 0.f, 0.f };
    float m_nearZ{ 0.1f };
    float m_farZ{ 1000.f };
    std::uint32_t m_projVersion{ 0 };

    mutable Core::Matrix m_view{ Core::Matrix::Identity() };
    mutable bool m_dirty{ true };
};