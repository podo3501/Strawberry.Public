export module Core.Math:Matrix;

import std;

namespace Core
{
    // 행 우선 저장이고 행 벡터에 곱한다 (v * M). 이동은 m[3][0..2]이고 왼손 좌표계이며 깊이는 0~1이다.
    export struct Matrix
    {
        float m[4][4]{ { 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f } };   // 기본은 단위 행렬이다

        static constexpr Matrix Identity() { return Matrix{}; }

        static constexpr Matrix Translation(float x, float y, float z)
        {
            Matrix r = Identity();
            r.m[3][0] = x;
            r.m[3][1] = y;
            r.m[3][2] = z;
            return r;
        }

        static constexpr Matrix Scale(float x, float y, float z)
        {
            Matrix r = Identity();
            r.m[0][0] = x;
            r.m[1][1] = y;
            r.m[2][2] = z;
            return r;
        }

        static Matrix RotationX(float rad)
        {
            Matrix r = Identity();

            float c = std::cos(rad);
            float s = std::sin(rad);

            r.m[1][1] = c;
            r.m[1][2] = s;
            r.m[2][1] = -s;
            r.m[2][2] = c;

            return r;
        }

        static Matrix RotationY(float rad)
        {
            Matrix r = Identity();

            float c = std::cos(rad);
            float s = std::sin(rad);

            r.m[0][0] = c;
            r.m[0][2] = -s;
            r.m[2][0] = s;
            r.m[2][2] = c;

            return r;
        }

        static Matrix RotationZ(float rad)
        {
            Matrix r = Identity();

            float c = std::cos(rad);
            float s = std::sin(rad);

            r.m[0][0] = c;
            r.m[0][1] = s;
            r.m[1][0] = -s;
            r.m[1][1] = c;

            return r;
        }

        constexpr Matrix operator*(const Matrix& rhs) const
        {
            Matrix result;

            for (int r = 0; r < 4; ++r)
            {
                for (int c = 0; c < 4; ++c)
                {
                    result.m[r][c] =
                        m[r][0] * rhs.m[0][c] +
                        m[r][1] * rhs.m[1][c] +
                        m[r][2] * rhs.m[2][c] +
                        m[r][3] * rhs.m[3][c];
                }
            }

            return result;
        }

        constexpr Matrix& operator*=(const Matrix& rhs)
        {
            *this = *this * rhs;
            return *this;
        }

        // 성분별 비교라서 +0.0과 -0.0은 같고 NaN은 자기 자신과도 다르다 (벡터, 색과 같다)
        constexpr bool operator==(const Matrix&) const = default;

        const float* Data() const { return &m[0][0]; }
        float* Data() { return &m[0][0]; }
    };
}