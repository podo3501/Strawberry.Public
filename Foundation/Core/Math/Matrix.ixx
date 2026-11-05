export module Core.Math:Matrix;

import std;

namespace Core
{
    // row-major
    export class Matrix
    {
    public:
        float m[4][4]{ 0.0f };

    public:
        constexpr Matrix()
        {
            SetIdentity();
        }

        // 컴파일러 기본 제공 복사 생성자 및 대입 연산자 (constexpr 지원)
        constexpr Matrix(const Matrix& rhs) = default;
        constexpr Matrix& operator=(const Matrix& rhs) = default;

        constexpr void SetIdentity()
        {
            std::fill(&m[0][0], &m[0][0] + 16, 0.0f);

            m[0][0] = 1.0f;
            m[1][1] = 1.0f;
            m[2][2] = 1.0f;
            m[3][3] = 1.0f;
        }

        static constexpr Matrix Identity()
        {
            Matrix r;
            r.SetIdentity();
            return r;
        }

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

        // 행렬 곱은 오른쪽부터 적용되므로 결과적으로 s -> r -> t 순으로 적용됨
        static Matrix TRS(const Matrix& t, const Matrix& r, const Matrix& s)
        {
            return t * r * s;
        }

        static constexpr Matrix OrthographicOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ)
        {
            Matrix r = Identity();

            float rl = right - left;
            float tb = top - bottom;
            float fn = farZ - nearZ;

            r.m[0][0] = 2.0f / rl;
            r.m[1][1] = 2.0f / tb;
            r.m[2][2] = 1.0f / fn;

            r.m[3][0] = -(left + right) / rl;
            r.m[3][1] = -(top + bottom) / tb;
            r.m[3][2] = -nearZ / fn;

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

        bool operator==(const Matrix& rhs) const
        {
            return std::memcmp(m, rhs.m, sizeof(m)) == 0;
        }

        bool operator!=(const Matrix& rhs) const
        {
            return !(*this == rhs);
        }

        const float* Data() const { return &m[0][0]; }
        float* Data() { return &m[0][0]; }
    };
}