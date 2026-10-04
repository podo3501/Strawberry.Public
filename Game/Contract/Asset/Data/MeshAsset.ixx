export module Client.Asset.Data:MeshAsset;

import std;
import Client.Asset.AssetData;
import Core.TypeHierarchy;
import Core.Math;

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