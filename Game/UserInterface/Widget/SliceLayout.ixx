export module Game.UserInterface.Widget:SliceLayout;

import std;
import Core.Math;

export struct SliceMargins // 소스 텍스처 픽셀 단위
{ 
	float left{ 0.f };
	float top{ 0.f };
	float right{ 0.f };
	float bottom{ 0.f };

	static constexpr SliceMargins NineSlice(float l, float t, float r, float b) { return { l, t, r, b }; }
	static constexpr SliceMargins Uniform(float m) { return { m, m, m, m }; }
	static constexpr SliceMargins HorizontalThree(float l, float r) { return { l, 0.f, r, 0.f }; } // 좌/우 고정, 가운데 가로로 늘어남
	static constexpr SliceMargins VerticalThree(float t, float b) { return { 0.f, t, 0.f, b }; } // 위/아래 고정, 가운데 세로로 늘어남
}; 

export struct SliceQuad 
{ 
	Core::Rect dest; 
	Core::Rect uv; 
};

export std::array<SliceQuad, 9> BuildNineSlice(
	const Core::Rect& dest,
	const SliceMargins& m,          // 소스 픽셀 기준 마진
	Core::Vector2 srcSize)          // 소스 이미지 크기(px)
{
	// dest가 마진 합보다 작으면 모서리를 비례 축소 (안 하면 쿼드가 뒤집힘)
	const float sx = (m.left + m.right > dest.width) ? dest.width / (m.left + m.right) : 1.f;
	const float sy = (m.top + m.bottom > dest.height) ? dest.height / (m.top + m.bottom) : 1.f;
	const float l = m.left * sx, r = m.right * sx;
	const float t = m.top * sy, b = m.bottom * sy;

	const float dx[4] = { dest.x, dest.x + l, dest.x + dest.width - r, dest.x + dest.width };
	const float dy[4] = { dest.y, dest.y + t, dest.y + dest.height - b, dest.y + dest.height };
	const float u[4] = { 0.f, m.left / srcSize.x, 1.f - m.right / srcSize.x, 1.f };
	const float v[4] = { 0.f, m.top / srcSize.y,  1.f - m.bottom / srcSize.y, 1.f };

	std::array<SliceQuad, 9> out{};
	for (int j = 0; j < 3; ++j)
		for (int i = 0; i < 3; ++i)
			out[j * 3 + i] = {
				{ dx[i], dy[j], dx[i + 1] - dx[i], dy[j + 1] - dy[j] },
				{ u[i],  v[j],  u[i + 1] - u[i],  v[j + 1] - v[j] } };

	return out;
}