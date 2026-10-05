export module Graphics.SceneObject:UICamera;

import std;
import :Camera;
import Core.Math;

export class UICamera : public Camera
{
public:
    virtual ~UICamera() override = default;

    UICamera()
    {
        SetNearFar(0.f, 1.f);
    }

    virtual Core::Matrix BuildProjection(
        const Core::Size& screenSize,
        const std::optional<Core::Rect>& viewport) const override
    {
        float width = m_width;
        float height = m_height;

        if (width <= 0.0f || height <= 0.0f)
        {
            if (viewport.has_value())
            {
                width = viewport->width;
                height = viewport->height;
            }
            else
            {
                width = static_cast<float>(screenSize.width);
                height = static_cast<float>(screenSize.height);
            }
        }

        if (width != m_lastWidth || height != m_lastHeight || GetProjVersion() != m_lastProjVersion)
        {
            m_proj = Core::Matrix::OrthographicOffCenter(0.f, width, height, 0.f, m_nearZ, m_farZ);
            m_lastWidth = width;
            m_lastHeight = height;
            m_lastProjVersion = GetProjVersion();
        }
        return m_proj;
    }

    void SetOrthoSize(float width, float height) // 명시적으로 크기를 안 주면 BuildProjection이 screenSize/viewport로 자동 계산
    {
        m_width = width;
        m_height = height;

        MarkProjDirty();
    }

protected:
    virtual void UpdateMatrices() const override
    {
        // UI는 회전 없음. 패닝이 필요해지면 여기서 m_position 반영.
        m_view = Core::Matrix::Identity();
    }

private:
    float m_width{ 0.0f };
    float m_height{ 0.0f };

    mutable Core::Matrix m_proj;
    mutable float m_lastWidth{ -1.0f };
    mutable float m_lastHeight{ -1.0f };
    mutable std::uint32_t m_lastProjVersion{ 0xFFFFFFFF };
};