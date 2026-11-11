export module Core.Math:CameraMatrices;

import std;
import :Vector3;
import :Matrix;
import Core.Assert;

namespace Core
{
    // 눈과 목표가 같으면 +Z를 보고, 위 방향이 보는 방향과 나란하면 +X를 오른쪽 축으로 쓴다 (축이 0 벡터가 되지 않는다)
    export Matrix CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up)
    {
        Vector3 zaxis = (target - eye).NormalizedOr(Vector3(0.0f, 0.0f, 1.0f));
        Vector3 xaxis = up.Cross(zaxis).NormalizedOr(Vector3(1.0f, 0.0f, 0.0f));
        Vector3 yaxis = zaxis.Cross(xaxis);

        Matrix r;

        r.m[0][0] = xaxis.x;
        r.m[0][1] = yaxis.x;
        r.m[0][2] = zaxis.x;
        r.m[0][3] = 0.0f;

        r.m[1][0] = xaxis.y;
        r.m[1][1] = yaxis.y;
        r.m[1][2] = zaxis.y;
        r.m[1][3] = 0.0f;

        r.m[2][0] = xaxis.z;
        r.m[2][1] = yaxis.z;
        r.m[2][2] = zaxis.z;
        r.m[2][3] = 0.0f;

        r.m[3][0] = -xaxis.Dot(eye);
        r.m[3][1] = -yaxis.Dot(eye);
        r.m[3][2] = -zaxis.Dot(eye);
        r.m[3][3] = 1.0f;

        return r;
    }

    // 너비, 높이, 깊이가 0이면 결과가 없으므로 Assert다 (호출하는 쪽이 크기 0일 때 부르지 않는다)
    export constexpr Matrix CreateOrthographicOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ)
    {
        Assert(right != left && top != bottom && farZ != nearZ);
        Matrix r = Matrix::Identity();

        const float rl = right - left;
        const float tb = top - bottom;
        const float fn = farZ - nearZ;

        r.m[0][0] = 2.0f / rl;
        r.m[1][1] = 2.0f / tb;
        r.m[2][2] = 1.0f / fn;

        r.m[3][0] = -(left + right) / rl;
        r.m[3][1] = -(top + bottom) / tb;
        r.m[3][2] = -nearZ / fn;

        return r;
    }

    // aspect가 0이거나 zn과 zf가 같거나 fovY가 0이면 결과가 없으므로 Assert다 (호출하는 쪽이 크기 0일 때 부르지 않는다)
    export Matrix CreatePerspectiveFov(float fovY, float aspect, float zn, float zf)
    {
        Assert(aspect != 0.0f && zf != zn && std::tan(fovY * 0.5f) != 0.0f);
        Matrix r{};

        float yScale = 1.0f / std::tan(fovY * 0.5f);
        float xScale = yScale / aspect;

        r.m[0][0] = xScale;
        r.m[0][1] = 0.0f;
        r.m[0][2] = 0.0f;
        r.m[0][3] = 0.0f;

        r.m[1][0] = 0.0f;
        r.m[1][1] = yScale;
        r.m[1][2] = 0.0f;
        r.m[1][3] = 0.0f;

        r.m[2][0] = 0.0f;
        r.m[2][1] = 0.0f;
        r.m[2][2] = zf / (zf - zn);
        r.m[2][3] = 1.0f;

        r.m[3][0] = 0.0f;
        r.m[3][1] = 0.0f;
        r.m[3][2] = (-zn * zf) / (zf - zn);
        r.m[3][3] = 0.0f;

        return r;
    }
}