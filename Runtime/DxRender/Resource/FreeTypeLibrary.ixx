module;

#include <ft2build.h>
#include FT_FREETYPE_H

export module DxRender.Resource:FreeTypeLibrary;

import std;
import Core.Assert;

export class FreeTypeLibrary
{
public:
    ~FreeTypeLibrary()
    {
        if (m_library)
        {
            FT_Done_FreeType(m_library);
            m_library = nullptr;
        }
    }

    FreeTypeLibrary()
    {
        FT_Error err = FT_Init_FreeType(&m_library);
        Core::Assert(err == FT_Err_Ok);
    }

    FreeTypeLibrary(const FreeTypeLibrary&) = delete;
    FreeTypeLibrary& operator=(const FreeTypeLibrary&) = delete;

    FreeTypeLibrary(FreeTypeLibrary&& other) noexcept :
        m_library(std::exchange(other.m_library, nullptr))
    {}

    FreeTypeLibrary& operator=(FreeTypeLibrary&& other) noexcept
    {
        if (this != &other)
        {
            if (m_library)
            {
                FT_Done_FreeType(m_library);
            }
            m_library = std::exchange(other.m_library, nullptr);
        }
        return *this;
    }

    FT_Library Get() const noexcept { return m_library; }

private:
    FT_Library m_library{ nullptr };
};