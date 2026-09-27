export module Client.Asset.Data:SoundTable;

import std;
import :AudioTypes;
import Client.Asset.AssetData;
import Core.ResourceID;
import Core.TypeHierarchy;

export class Serializer;
export struct SoundDesc
{
	virtual ~SoundDesc() = default;
	SoundDesc() = default;
	explicit SoundDesc(SoundType _sndType) : sndType(_sndType) {}

	void Serialize(Serializer& serializer);

	SoundType sndType;
	Core::ResourceID resID;
	AudioGroup group;
	int priority{ 0 };
	float volume{ 0.f };
};

export struct StaticSoundDesc : public SoundDesc
{
	StaticSoundDesc() : SoundDesc{ SoundType::Static } {}
	void Serialize(Serializer& serializer);

	//static 항목이 생기면 여기에 추가
};

export struct StaticSoundTable : public Core::TypeNode<StaticSoundTable, AssetData>
{
	virtual ~StaticSoundTable() = default;

	const StaticSoundDesc* GetDescriptor(std::string_view index) const noexcept;
	void Serialize(Serializer& serializer);

private:
	std::unordered_map<std::string, StaticSoundDesc> m_descs;
};

export struct StreamSoundDesc : public SoundDesc
{
	StreamSoundDesc() : SoundDesc{ SoundType::Stream } {}
	void Serialize(Serializer& serializer);

	bool loop{ false };
};

export struct StreamSoundTable : public Core::TypeNode<StreamSoundTable, AssetData>
{
	virtual ~StreamSoundTable() = default;

	const StreamSoundDesc* GetDescriptor(std::string_view index) const noexcept;
	void Serialize(Serializer& serializer);

private:
	std::unordered_map<std::string, StreamSoundDesc> m_descs;
};

export struct SoundAssetView
{
	std::shared_ptr<StaticSoundTable> staticSoundTable;
	std::shared_ptr<StreamSoundTable> streamSoundTable;
};
