export module Runtime.Render.RGResourceID:Types;

import std;

export using RGResourceID = std::uint32_t;
export inline constexpr RGResourceID InvalidRGID = std::numeric_limits<RGResourceID>::max();