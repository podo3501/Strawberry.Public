#include <nlohmann/json.hpp>

import std;
import Serialization;
import Core.Utils;
import Core.AssetPath;
import Core.ResourceID;
import Core.BasicTypes;
import Core.Logger;
import Contract.Common.Asset;
import Contract.Audio.Assets;
import Contract.Asset.Backend;

namespace
{
	// JSON 소리 표를 읽는 로더. JSON 문법이나 값의 형식이 틀리면 Error를 남기고 nullptr
	template<typename T>
	class SoundTableLoader : public IAssetLoader
	{
	public:
		~SoundTableLoader() override = default;

		std::shared_ptr<Asset> Load(AssetInput& source) override
		{
			if (source.IsStream()) return nullptr;

			auto& mem = static_cast<MemoryInput&>(source);
			return LoadFromMemory(mem.buffer);
		}

	private:
		std::shared_ptr<Asset> LoadFromMemory(const Core::ByteBuffer& buffer)
		{
			const nlohmann::json data = nlohmann::json::parse(buffer.begin(), buffer.end(), nullptr, false);   // 예외 없이 실패를 알린다
			if (data.is_discarded())
			{
				Core::Logger::Error("SoundTableLoader: JSON 문법이 올바르지 않음");
				return nullptr;
			}

			auto table = std::make_shared<T>();
			auto reader = Serializer::ForRead(data);
			table->Serialize(reader);
			if (reader.HasError())
			{
				Core::Logger::Error("SoundTableLoader: 소리 표를 읽지 못함 (위의 Serializer 오류 참고)");
				return nullptr;
			}

			return table;
		}
	};
}

template<>
constexpr std::array<const char*, Core::EnumSize<SoundType>> Core::EnumToStringMap<SoundType>()
{
	return Core::MakeEnumStringMap<SoundType>("Static", "Stream");
}

template<>
constexpr std::array<const char*, Core::EnumSize<AudioGroup>> Core::EnumToStringMap<AudioGroup>()
{
	return Core::MakeEnumStringMap<AudioGroup>("BGM", "SFX", "UI", "System");
}

////////////////////////////////////////////////////////////////

// 파일 이름은 저장소 안의 상대 경로이고 확장자가 있어야 한다 (ResourceID::MakePath가 확장자를 요구한다)
template<>
struct JsonTraits<Core::ResourceID>
{
	static std::optional<nlohmann::json> SerializeToJson(const Core::ResourceID& data)
	{
		return nlohmann::json(data.GetValue());
	}

	static std::optional<Core::ResourceID> DeserializeFromJson(const nlohmann::json& dataJ)
	{
		if (!dataJ.is_string())
			return std::nullopt;

		const auto path = Core::AssetPath::TryMake(dataJ.get<std::string>());
		if (!path || !path->HasExtension())
			return std::nullopt;

		return Core::ResourceID::MakePath(*path);
	}
};

////////////////////////////////////////////////////////////////

void SoundDesc::Serialize(Serializer& serializer)
{
	serializer.Process("Filename", resID);
	serializer.Process("Group", group);
	serializer.Process("Priority", priority);
	serializer.Process("Volume", volume);

	if (serializer.IsWrite())
		return;

	// 읽은 값의 의미를 검사한다. Group이 없으면 InvalidEnum으로 남고 서비스가 거른다 (모르는 이름은 읽기 오류)
	if (resID.GetType() != Core::ResourceIDType::Path)
		serializer.ReportError("Filename이 필요함");
	if (priority < 0)
		serializer.ReportError("Priority는 0 이상이어야 함");
	if (!(volume >= 0.f && volume <= 1.f))
		serializer.ReportError("Volume은 0에서 1 사이여야 함");
}

////////////////////////////////////////////////////////////////

void StaticSoundDesc::Serialize(Serializer& serializer)
{
	SoundDesc::Serialize(serializer);

	//static 항목이 생기면 여기에 추가
}

const StaticSoundDesc* StaticSoundTable::GetDescriptor(std::string_view index) const noexcept
{
	auto it = m_descs.find(std::string(index));
	if (it != m_descs.end())
		return &it->second;

	return nullptr;
}

void StaticSoundTable::Serialize(Serializer& serializer)
{
	serializer.Process("Descriptors", m_descs);
}

std::unique_ptr<IAssetLoader> CreateStaticSoundTableLoader()
{
	return std::make_unique<SoundTableLoader<StaticSoundTable>>();
}

////////////////////////////////////////////////////////////////

void StreamSoundDesc::Serialize(Serializer& serializer)
{
	SoundDesc::Serialize(serializer);

	serializer.Process("Loop", loop);
}

const StreamSoundDesc* StreamSoundTable::GetDescriptor(std::string_view index) const noexcept
{
	auto it = m_descs.find(std::string(index));
	if (it != m_descs.end())
		return &it->second;

	return nullptr;
}

void StreamSoundTable::Serialize(Serializer& serializer)
{
	serializer.Process("Descriptors", m_descs);
}

std::unique_ptr<IAssetLoader> CreateStreamSoundTableLoader()
{
	return std::make_unique<SoundTableLoader<StreamSoundTable>>();
}