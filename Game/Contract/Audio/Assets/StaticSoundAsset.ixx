export module Contract.Audio.Assets:StaticSound;

import std;
import Contract.Common.Asset;
import Core.BasicTypes;
import Core.TypeHierarchy;

export enum class SampleFormat
{
	Int16,
	Float32
};

export struct StaticSoundAsset : public Core::TypeNode<StaticSoundAsset, Asset>
{
	virtual ~StaticSoundAsset() = default;

	int sampleRate{ 0 };
	int channels{ 0 };
	SampleFormat format{ SampleFormat::Int16 };

	std::vector<std::uint8_t> data;

	size_t SampleCount() const
	{
		size_t bytesPerSample = (format == SampleFormat::Int16) ? sizeof(std::int16_t) : sizeof(float);
		return data.size() / bytesPerSample;
	}
};