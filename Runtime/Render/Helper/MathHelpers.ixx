module;

#include <DirectXMath.h>

export module Runtime.Render.Helper:Math;

import std;
import Core.Math;

export inline DirectX::XMFLOAT3 ToXMFLOAT3(const Core::Vector3& vec)
{
    return DirectX::XMFLOAT3(vec.x, vec.y, vec.z);
}

export inline DirectX::XMFLOAT4 ToXMFLOAT4(const Core::Vector4& vec)
{
    return DirectX::XMFLOAT4(vec.x, vec.y, vec.z, vec.w);
}

export inline DirectX::XMMATRIX ToDXMatrix(const Core::Matrix& m)
{
    return DirectX::XMMATRIX(
        m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
        m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
        m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
        m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
}