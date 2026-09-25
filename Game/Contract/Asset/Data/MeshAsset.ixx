export module Client.Asset.Data:MeshAsset;

import std;
import :AssetData;
import Core.TypeHierarchy;
import Core.Math;

namespace Client
{
	export enum class VertexFormat
	{
		Mesh,
		UI,
		Grid,
	};

	export struct MeshVertex
	{
		Core::Vector3 position;
		Core::Vector3 normal;
		Core::Vector2 uv;
		Core::Vector3 tangent;
	};

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

	export struct MeshAsset : public Core::TypeNode<MeshAsset, AssetData>
	{
		virtual ~MeshAsset() = default;

		VertexFormat format{ VertexFormat::Mesh };

		std::uint32_t vertexStride{ 0 }; //generic하게 byte로 바뀌었기 때문에 보폭을 저장해야 한다.
		std::uint32_t vertexCount{ 0 };

		std::vector<std::byte> vertices; //ui vertex, mesh vertex 같이 struct 가 달라도 다 들어가게끔 generic 하게.
		std::vector<std::uint32_t> indices;

		template<typename T>
		void SetVertices(const std::vector<T>& src)
		{
			vertexStride = sizeof(T);
			vertexCount = static_cast<std::uint32_t>(src.size());

			vertices.resize(sizeof(T) * src.size());
			std::memcpy(vertices.data(), src.data(), vertices.size());
		}
	};
}