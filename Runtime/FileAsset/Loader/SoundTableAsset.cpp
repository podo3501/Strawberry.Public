module;

#include <nlohmann/json.hpp>

module Contract.Asset.Data:SoundTable;

import std;
import Core.Utils;
import Core.ResourceID;
import Contract.Asset;
import FileAsset.JsonLoader;
import Runtime.Serialization;

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

nlohmann::json JsonTraitsBase<SoundType>::SerializeToJson(const SoundType& data)
{
	return Core::EnumToString(data);
}

SoundType JsonTraitsBase<SoundType>::DeserializeFromJson(const nlohmann::json& dataJ)
{
	return CreateAndFill<SoundType>([&dataJ](SoundType& data) {
		data = *Core::StringToEnum<SoundType>(dataJ.get<std::string>());
		});
}

nlohmann::json JsonTraitsBase<Core::ResourceID>::SerializeToJson(const Core::ResourceID& data)
{
	return data.GetValue();
}

Core::ResourceID JsonTraitsBase<Core::ResourceID>::DeserializeFromJson(const nlohmann::json& dataJ)
{
	return Core::ResourceID::MakePath(dataJ.get<std::string>());
}

nlohmann::json JsonTraitsBase<AudioGroup>::SerializeToJson(const AudioGroup& data)
{
	return Core::EnumToString(data);
}

AudioGroup JsonTraitsBase<AudioGroup>::DeserializeFromJson(const nlohmann::json& dataJ)
{
	return CreateAndFill<AudioGroup>([&dataJ](AudioGroup& data) {
		data = *Core::StringToEnum<AudioGroup>(dataJ.get<std::string>());
		});
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
	return CreateJsonLoader<StaticSoundTable>();
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
	return CreateJsonLoader<StreamSoundTable>();
}