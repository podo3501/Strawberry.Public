#include <nlohmann/json.hpp>

import std;
import Serialization;
import Core.Utils;
import Core.ResourceID;
import Core.BasicTypes;
import Core.Logger;
import Contract.Common.Asset;
import Contract.Audio.Assets;
import Contract.Asset.Backend;

namespace
{
	// JSON 소리 표를 읽는 로더. JSON 문법이 틀리면 Error를 남기고 nullptr
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
			DeserializeClass(data, *table);
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

nlohmann::json JsonTraitsBase<Core::ResourceID>::SerializeToJson(const Core::ResourceID& data)
{
	return data.GetValue();
}

Core::ResourceID JsonTraitsBase<Core::ResourceID>::DeserializeFromJson(const nlohmann::json& dataJ)
{
	return Core::ResourceID::MakePath(dataJ.get<std::string>());
}

////////////////////////////////////////////////////////////////

void SoundDesc::Serialize(Serializer& serializer)
{
	serializer.Process("Filename", resID);
	serializer.Process("Group", group);
	serializer.Process("Priority", priority);
	serializer.Process("Volume", volume);
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