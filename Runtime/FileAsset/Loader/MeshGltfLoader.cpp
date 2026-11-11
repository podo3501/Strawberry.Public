#pragma warning(push)
#pragma warning(disable : 4996)
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#pragma warning(pop)

import std;
import Core.Type;
import Core.Logger;
import Contract.Common.Asset;
import Contract.Render.Assets;
import Contract.Asset.Service;
import Contract.Asset.Backend;

namespace
{
	struct CgltfDeleter
	{
		void operator()(cgltf_data* data) const { cgltf_free(data); }
	};
	using CgltfPtr = std::unique_ptr<cgltf_data, CgltfDeleter>;

	// 접근자의 모든 요소를 dst에 stride 간격으로 읽는다. dst가 담을 수 있는 개수(maxCount)를 넘으면 읽지 않고 실패한다
	bool ReadAccessorFloats(const cgltf_accessor* accessor, float* dst, size_t componentCount, size_t dstStride, size_t maxCount)
	{
		if (!accessor || accessor->count > maxCount) return false;

		size_t count = accessor->count;
		for (size_t i = 0; i < count; ++i)
		{
			float temp[4] = {};
			if (!cgltf_accessor_read_float(accessor, i, temp, componentCount))
				return false;

			float* dstElem = reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(dst) + i * dstStride);
			for (size_t c = 0; c < componentCount; ++c)
				dstElem[c] = temp[c];
		}
		return true;
	}

	// glTF는 TANGENT를 없이 export하는 경우가 흔해서, 없으면 임의의 기본값으로 채움
	void FillDefaultTangents(std::vector<MeshVertex>& vertices)
	{
		for (auto& v : vertices)
			v.tangent = { 1.0f, 0.0f, 0.0f };
	}

	// 속성 접근자가 기대한 형식(요소 종류)이고 정점 수와 같은 개수인지 확인한다
	bool CheckAttribute(const cgltf_accessor* accessor, cgltf_type type, size_t vertexCount, const char* name)
	{
		if (!accessor)
			return true;   // 없는 속성은 기본값으로 채운다
		if (accessor->type != type || accessor->count != vertexCount)
		{
			Core::Logger::Error("MeshGltfLoader: {} 접근자의 형식이나 개수가 맞지 않음 (개수 {}, 정점 {})", name, accessor->count, vertexCount);
			return false;
		}
		return true;
	}
}

class MeshGltfLoader : public IAssetLoader
{
public:
	~MeshGltfLoader() override = default;
	explicit MeshGltfLoader(IAssetProvider* assetProvider) noexcept
		: m_assetProvider{ assetProvider }
	{}

	std::shared_ptr<Asset> Load(AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<MemoryInput&>(source);
		return LoadFromMemory(mem.resID, std::move(mem.buffer));
	}

private:
	std::shared_ptr<MeshAsset> LoadFromMemory(const Core::ResourceID& resID, Core::ByteBuffer buffer)
	{
		cgltf_options options{};
		cgltf_data* parsed = nullptr;

		// JSON 파싱만 함 - 아직 버퍼(.bin)나 이미지는 안 건드림
		cgltf_result result = cgltf_parse(&options, buffer.data(), buffer.size(), &parsed);
		if (result != cgltf_result_success || !parsed)
		{
			Core::Logger::Error("MeshGltfLoader: glTF를 읽지 못함 (cgltf 오류 {})", static_cast<int>(result));
			return nullptr;
		}
		CgltfPtr data{ parsed };

		// 버퍼는 .gltf 파일 옆의 .bin 파일만 지원한다 (내장 버퍼와 data URI는 지원하지 않음)
		std::vector<Core::ByteBuffer> loadedBuffers;
		loadedBuffers.reserve(data->buffers_count);   // 아래에서 data 포인터를 잡으므로 다시 할당되면 안 된다

		for (size_t i = 0; i < data->buffers_count; ++i)
		{
			cgltf_buffer& buf = data->buffers[i];

			if (!buf.uri || std::string_view{ buf.uri }.starts_with("data:"))
			{
				Core::Logger::Error("MeshGltfLoader: 버퍼는 옆의 .bin 파일만 지원함 (내장 버퍼와 data URI는 지원하지 않음)");
				return nullptr;
			}

			cgltf_decode_uri(buf.uri);   // %20 같은 표기를 되돌린다

			const auto binID = resID.TryMakeSibling(buf.uri);
			if (!binID)
			{
				Core::Logger::Error("MeshGltfLoader: 버퍼 파일 이름이 올바르지 않음 ({})", buf.uri);
				return nullptr;
			}

			auto binary = Core::Cast<BinaryAsset>(m_assetProvider->Load(Core::GetTypeID<BinaryAsset>(), *binID));
			if (!binary)
			{
				Core::Logger::Error("MeshGltfLoader: 버퍼 파일을 읽지 못함 ({})", binID->GetValue());
				return nullptr;
			}
			if (binary->buffer.size() < buf.size)
			{
				Core::Logger::Error("MeshGltfLoader: 버퍼 파일이 glTF가 말한 크기보다 짧음 ({} < {})", binary->buffer.size(), buf.size);
				return nullptr;
			}

			loadedBuffers.emplace_back(std::move(binary->buffer));
			buf.data = loadedBuffers.back().data();
		}

		// 접근자와 뷰가 버퍼 범위 안인지 확인한다. 이걸 통과해야 아래에서 안전하게 읽는다
		if (cgltf_validate(data.get()) != cgltf_result_success)
		{
			Core::Logger::Error("MeshGltfLoader: glTF 검증에 실패함 (접근자나 뷰가 버퍼를 벗어남)");
			return nullptr;
		}

		if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0)
		{
			Core::Logger::Error("MeshGltfLoader: 메시가 없음");
			return nullptr;
		}
		if (data->meshes_count > 1 || data->meshes[0].primitives_count > 1)
			Core::Logger::Warn("MeshGltfLoader: 첫 번째 메시의 첫 번째 프리미티브만 읽음 (메시 {}개, 프리미티브 {}개)", data->meshes_count, data->meshes[0].primitives_count);

		const cgltf_primitive& prim = data->meshes[0].primitives[0];
		if (prim.type != cgltf_primitive_type_triangles)
		{
			Core::Logger::Error("MeshGltfLoader: 삼각형 목록만 지원함 (프리미티브 종류 {})", static_cast<int>(prim.type));
			return nullptr;
		}

		const cgltf_accessor* posAccessor = nullptr;
		const cgltf_accessor* normalAccessor = nullptr;
		const cgltf_accessor* uvAccessor = nullptr;
		const cgltf_accessor* tangentAccessor = nullptr;

		for (size_t i = 0; i < prim.attributes_count; ++i)
		{
			const cgltf_attribute& attr = prim.attributes[i];
			if (attr.index != 0)
				continue;   // 두 번째 UV 같은 것은 쓰지 않는다

			switch (attr.type)
			{
			case cgltf_attribute_type_position: posAccessor = attr.data; break;
			case cgltf_attribute_type_normal:   normalAccessor = attr.data; break;
			case cgltf_attribute_type_texcoord: uvAccessor = attr.data; break;
			case cgltf_attribute_type_tangent:  tangentAccessor = attr.data; break;
			default: break;
			}
		}

		if (!posAccessor || posAccessor->count == 0)
		{
			Core::Logger::Error("MeshGltfLoader: POSITION이 없거나 비어 있음");
			return nullptr;
		}

		const size_t vertexCount = posAccessor->count;
		if (!CheckAttribute(posAccessor, cgltf_type_vec3, vertexCount, "POSITION") ||
			!CheckAttribute(normalAccessor, cgltf_type_vec3, vertexCount, "NORMAL") ||
			!CheckAttribute(uvAccessor, cgltf_type_vec2, vertexCount, "TEXCOORD_0") ||
			!CheckAttribute(tangentAccessor, cgltf_type_vec4, vertexCount, "TANGENT"))
			return nullptr;

		std::vector<MeshVertex> vertices(vertexCount);

		if (!ReadAccessorFloats(posAccessor, &vertices[0].position.x, 3, sizeof(MeshVertex), vertexCount) ||
			(normalAccessor && !ReadAccessorFloats(normalAccessor, &vertices[0].normal.x, 3, sizeof(MeshVertex), vertexCount)) ||
			(uvAccessor && !ReadAccessorFloats(uvAccessor, &vertices[0].uv.x, 2, sizeof(MeshVertex), vertexCount)))
		{
			Core::Logger::Error("MeshGltfLoader: 정점 속성을 읽지 못함");
			return nullptr;
		}

		if (tangentAccessor)
		{
			for (size_t i = 0; i < vertexCount; ++i)
			{
				float temp[4] = {};
				if (!cgltf_accessor_read_float(tangentAccessor, i, temp, 4))
				{
					Core::Logger::Error("MeshGltfLoader: TANGENT를 읽지 못함");
					return nullptr;
				}
				vertices[i].tangent = { temp[0], temp[1], temp[2] };
			}
		}
		else
		{
			FillDefaultTangents(vertices);
		}

		std::vector<uint32_t> indices;
		if (prim.indices)
		{
			if (prim.indices->count % 3 != 0)
			{
				Core::Logger::Error("MeshGltfLoader: 인덱스 개수가 3의 배수가 아님 ({})", prim.indices->count);
				return nullptr;
			}

			indices.resize(prim.indices->count);
			for (size_t i = 0; i < prim.indices->count; ++i)
			{
				const cgltf_size index = cgltf_accessor_read_index(prim.indices, i);
				if (index >= vertexCount)
				{
					Core::Logger::Error("MeshGltfLoader: 인덱스가 정점 개수를 넘음 ({} >= {})", index, vertexCount);
					return nullptr;
				}
				indices[i] = static_cast<uint32_t>(index);
			}
		}
		else if (vertexCount % 3 != 0)
		{
			Core::Logger::Error("MeshGltfLoader: 인덱스 없는 삼각형 목록인데 정점 개수가 3의 배수가 아님 ({})", vertexCount);
			return nullptr;
		}

		auto mesh = std::make_shared<MeshAsset>();
		mesh->format = VertexFormat::Mesh;
		mesh->SetVertices(vertices);
		mesh->indices = std::move(indices);

		return mesh;
	}

	IAssetProvider* m_assetProvider{ nullptr };
};

std::unique_ptr<IAssetLoader> CreateMeshGltfLoader(IAssetProvider* assetProvider)
{
	return std::make_unique<MeshGltfLoader>(assetProvider);
}