export module Contract.Asset.Data:StaticSound;

import std;
import Contract.Asset.AssetData;
import Core.Types;
import Core.TypeHierarchy;

export enum class SampleFormat
{
	Int16,
	Float32
};

export struct StaticSoundAsset : public Core::TypeNode<StaticSoundAsset, AssetData>
{
	virtual ~StaticSoundAsset() = default;

	int sampleRate{ 0 };
	int channels{ 0 };
	SampleFormat format{ SampleFormat::Int16 };

	std::vector<std::uint8_t> data; //?!? 추후에 Core::ByteBuffer 로 바꾸자.

	size_t SampleCount() const
	{
		size_t bytesPerSample = (format == SampleFormat::Int16) ? sizeof(std::int16_t) : sizeof(float);
		return data.size() / bytesPerSample;
	}
};