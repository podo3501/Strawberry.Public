export module Runtime.Render.RGResourceID:Generator;

import std;
import :Types;
import Core.Assert;

export class RGResourceIDGenerator
{
public:
    void Reset() noexcept
    {
        m_next = 0;
    }

    RGResourceID Generate() noexcept
    {
        if (m_next == InvalidRGID)
        {
            Core::Assert(false); // RGResourceID의 최대 범위를 넘어섰다.
            return InvalidRGID;
        }

        return m_next++;
    }

    std::uint32_t Count() const noexcept
    {
        return m_next;
    }

private:
    RGResourceID m_next{ 0 };
};