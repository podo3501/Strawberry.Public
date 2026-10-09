export module Graphics.SceneObject:DirectionalLight;

import std;
import Core.Math;
import Contract.Render.View;

export class DirectionalLight
{
public:
    DirectionalLight() = default;
    ~DirectionalLight() = default;

    void SetDirection(const Core::Vector3& dir)
    {
        m_direction = dir.NormalizedOr({ 0.0f, -1.0f, 0.0f });
    }

    void SetColor(const Core::Vector3& color)
    {
        m_color = color;
    }

    void SetIntensity(float intensity)
    {
        m_intensity = intensity;
    }

    DirectionalLightData BuildLightData() const
    {
        DirectionalLightData data{};
        data.direction = m_direction;
        data.color = m_color;
        data.intensity = m_intensity;

        Core::Vector3 targetCenter{ 0.0f, 0.0f, 0.0f };
        float lightDistance = 200.0f; // 씬 크기에 맞게 조절 가능
        Core::Vector3 lightPos = targetCenter - (m_direction * lightDistance);

        // 2. Light View Matrix 계산 (LookAt 변환 직접 유도) 기저 벡터 생성 (조명이 바라보는 방향이 곧 Forward 축)
        Core::Vector3 zAxis = m_direction.NormalizedOr({ 0.0f, -1.0f, 0.0f });

        // 업 벡터 기준 설정 (만약 조명이 수직으로 정방향 하강하면 임시로 Z축을 업벡터로 변경)
        Core::Vector3 upBasis{ 0.0f, 1.0f, 0.0f };
        if (std::abs(zAxis.y) > 0.99f)
            upBasis = Core::Vector3{ 0.0f, 0.0f, 1.0f };

        Core::Vector3 xAxis = upBasis.Cross(zAxis).NormalizedOr(Core::Vector3::Right());
        Core::Vector3 yAxis = zAxis.Cross(xAxis).NormalizedOr(Core::Vector3::Up());

        // Row-Major 카메라 뷰 변환 행렬 구성
        Core::Matrix lightView = Core::Matrix::Identity();
        lightView.m[0][0] = xAxis.x; lightView.m[1][0] = xAxis.y; lightView.m[2][0] = xAxis.z;
        lightView.m[0][1] = yAxis.x; lightView.m[1][1] = yAxis.y; lightView.m[2][1] = yAxis.z;
        lightView.m[0][2] = zAxis.x; lightView.m[1][2] = zAxis.y; lightView.m[2][2] = zAxis.z;

        // 카메라 이동(Translation) 성분 반영 (-Eye ∙ Axis)
        lightView.m[3][0] = -lightPos.Dot(xAxis);
        lightView.m[3][1] = -lightPos.Dot(yAxis);
        lightView.m[3][2] = -lightPos.Dot(zAxis);

        // 3. Light Projection Matrix 계산 (Orthographic) 조명이 커버할 공간의 가로세로 범위
        float shadowSizeX = 20.0f;
        float shadowSizeY = 20.0f;

        float halfWidth = shadowSizeX * 0.5f;
        float halfHeight = shadowSizeY * 0.5f;

        float nearZ = 0.1f;
        float farZ = 400.0f; // lightDistance 보다 커야 타겟 오브젝트들이 완전히 포함됨

        Core::Matrix lightProj = Core::Matrix::OrthographicOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight, nearZ, farZ);

        // 4. View * Proj 결합 후 구조체에 할당
        Core::Matrix lightViewProj = lightView * lightProj;
        data.viewProj = lightViewProj;

        return data;
    }

    const Core::Vector3& GetDirection() const { return m_direction; }
    const Core::Vector3& GetColor() const { return m_color; }
    float GetIntensity() const { return m_intensity; }

private:
    Core::Vector3 m_direction{ 0.0f, -1.0f, 0.0f };
    Core::Vector3 m_color{ 1.0f, 1.0f, 1.0f };
    float m_intensity{ 1.0f };
};