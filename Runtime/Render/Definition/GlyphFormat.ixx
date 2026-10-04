module;

#include <dxgiformat.h>

export module Runtime.Render.Definition:GlyphFormat;

import std;
import Core.Assert;

export enum class GlyphFormat
{
    R8,
    RG8,
    RGBA8,
};

export inline std::uint32_t GetBytesPerPixel(GlyphFormat format)
{
    switch (format)
    {
    case GlyphFormat::R8: return 1;
    case GlyphFormat::RG8: return 2;
    case GlyphFormat::RGBA8: return 4;
    }

    Core::Unreachable();
}

export inline DXGI_FORMAT GetDXGIFormat(GlyphFormat format)
{
    switch (format)
    {
    case GlyphFormat::R8: return DXGI_FORMAT_R8_UNORM;
    case GlyphFormat::RG8: return DXGI_FORMAT_R8G8_UNORM;
    case GlyphFormat::RGBA8: return DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    Core::Unreachable();
}

export struct GlyphPixels
{
    GlyphFormat format{ GlyphFormat::R8 };
    std::vector<std::uint8_t> buffer;

    std::uint32_t width{ 0 }; //atlas에 들어가는 크기 정보. 즉 uv 정보가 된다. 
    std::uint32_t height{ 0 };
};