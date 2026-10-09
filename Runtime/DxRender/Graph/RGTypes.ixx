export module DxRender.Graph:Types;

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