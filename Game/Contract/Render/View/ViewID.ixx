export module Contract.Render.View:ID;

import std;

export using ViewID = std::uint32_t;
export constexpr ViewID InvalidViewID = 0xFFFFFFFF;
export constexpr ViewID MaxViewCount = 10;