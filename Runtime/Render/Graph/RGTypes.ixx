export module Runtime.Render.Graph:Types;

import std;

export using PassIndex = int;
export using RGResourceID = std::uint32_t;
export inline constexpr RGResourceID InvalidRGID = std::numeric_limits<RGResourceID>::max();

export enum class RGAccess
{
	CopyDest,   // init / upload
	SRV,        // shader read
	UAV,        // unordered write/read
	RTV,        // backbuffer

	DepthWrite,
	DepthRead,

	Present     // present
};