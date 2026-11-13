export module DxRender.Graph:Types;

export enum class RGAccess
{
	CopyDest,   // init / upload
	SRV,        // 픽셀 셰이더가 읽음
	SRVNonPixel, // 정점/컴퓨트 셰이더가 읽음 (메시 버퍼 등)
	UAV,        // unordered write/read
	RTV,        // backbuffer

	DepthWrite,
	DepthRead,

	Present     // present
};