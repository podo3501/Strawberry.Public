export module DxRender.Task:Types;

import std;

export using LocalTaskID = std::uint32_t;
export constexpr LocalTaskID InvalidLocalTaskID = std::numeric_limits<LocalTaskID>::max();