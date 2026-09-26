#include <nlohmann/json.hpp>

import std;
import Core.Types;
import Client.Asset.Contract;
import Runtime.Serialization;

namespace
{
	struct JsonVec2
	{
		float x, y;

		void Serialize(Serializer& serializer)
		{
			serializer.Process("x", x);
			serializer.Process("y", y);
		}
	};

	struct JsonVec3
	{
		float x, y, z;

		void Serialize(Serializer& serializer)
		{
			serializer.Process("x", x);
			serializer.Process("y", y);
			serializer.Process("z", z);
		}
	};

	struct JsonMeshVertex
	{
		JsonVec3 position;
		JsonVec3 normal;
		JsonVec2 uv;
		JsonVec3 tangent;

		void Serialize(Serializer& serializer)
		{
			serializer.Process("position", position);
			serializer.Process("normal", normal);
			serializer.Process("uv", uv);
			serializer.Process("tangent", tangent);
		}
	};

	struct JsonMesh
	{
		std::string vertexFormat;
		std::vector<JsonMeshVertex> vertices;
		std::vector<uint32_t> indices;

		void Serialize(Serializer& serializer)
		{
			serializer.Process("vertexFormat", vertexFormat);
			serializer.Process("vertices", vertices);
			serializer.Process("indices", indices);
		}
	};

	VertexFormat ParseVertexFormat(const std::string& str)
	{
		if (str == "Mesh") return VertexFormat::Mesh;
		if (str == "UI")   return VertexFormat::UI;
		if (str == "Grid") return VertexFormat::Grid;

		return VertexFormat::Mesh;
	}

	MeshVertex ConvertToVertex(const JsonMeshVertex& v)
	{
		return {
			{ v.position.x, v.position.y, v.position.z },
			{ v.normal.x, v.normal.y, v.normal.z },
			{ v.uv.x, v.uv.y },
			{ v.tangent.x, v.tangent.y, v.tangent.z }
		};
	}
}

class MeshJsonLoader : public IAssetLoader
{
public:
	~MeshJsonLoader() override = default;

	std::shared_ptr<AssetData> Load(AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<const MemoryInput&>(source);
		return LoadFromMemory(mem.buffer);
	}

private:
	std::shared_ptr<MeshAsset> LoadFromMemory(const Core::ByteBuffer& buffer)
	{
		nlohmann::json rData = nlohmann::json::parse(buffer.begin(), buffer.end());
		
		JsonMesh jsonMesh;
		Serializer reader{ std::as_const(rData) };
		jsonMesh.Serialize(reader);

		auto mesh = std::make_shared<MeshAsset>();
		mesh->format = ParseVertexFormat(jsonMesh.vertexFormat);

		std::vector<MeshVertex> vertices;
		vertices.reserve(jsonMesh.vertices.size());

		for (const auto& v : jsonMesh.vertices)
			vertices.push_back(ConvertToVertex(v));

		mesh->SetVertices(vertices);
		mesh->indices = std::move(jsonMesh.indices);

		return mesh;
	}
};

std::unique_ptr<IAssetLoader> CreateMeshJsonLoader()
{
	return std::make_unique<MeshJsonLoader>();
}