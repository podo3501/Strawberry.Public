export module Client.Render.Definition:VertexTypes;

import std;
import Core.Math;

export enum class UIRenderMode : std::uint32_t
{
	UI = 0,
	BitmapText = 1,
	MTSDF = 2
};

export struct UITextProps
{
	float sdfPxRange{ 0.f };
	Core::Rect clipRect{
		-std::numeric_limits<float>::max() * 0.5f,
		-std::numeric_limits<float>::max() * 0.5f,
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max()
	}; // screen-space, clip 비활성 시 사실상 무제한

	std::uint32_t params1{ 0 }; //param1에 pack된 효과: outline, drop shadow, fill gradient
	std::uint32_t params2{ 0 }; //param2에 pack된 효과: glow
};

export struct UIVertex
{
	Core::Vector3 position;
	Core::Color color;
	Core::Vector2 uv;
	std::uint32_t textureIndex{ 0 }; //Bindless SRV Heap Index. 일단은 ui는 cb로 인덱스를 가지고 오는 걸로 사용한다.
	UIRenderMode mode{ UIRenderMode::UI };
	UITextProps textProps;
};

export struct GridVertex
{
	Core::Vector3 position;
	Core::Color color;
};