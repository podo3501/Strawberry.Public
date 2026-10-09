#pragma warning(push)
#pragma warning(disable: 4244)
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#pragma warning(pop)

import std;
import Core.Types;
import Contract.Asset;
import FileAsset.AudioProcessing;

class WavStaticLoader : public IAssetLoader
{
public:
	~WavStaticLoader() override = default;

	std::shared_ptr<AssetData> Load(AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<MemoryInput&>(source);
		return LoadFromMemory(std::move(mem.buffer));
	}

private:
	std::shared_ptr<StaticSoundAsset> LoadFromMemory(Core::ByteBuffer buffer)
	{
		drwav wav;

		if (!drwav_init_memory(&wav, buffer.data(), buffer.size(), nullptr))
			return nullptr;

		std::uint32_t channels = wav.channels;
		std::uint32_t sampleRate = wav.sampleRate;
		std::uint64_t totalFrames = wav.totalPCMFrameCount;

		if (channels == 0 || sampleRate == 0 || totalFrames == 0)
		{
			drwav_uninit(&wav);
			return nullptr;
		}

		std::vector<float> floatSamples;
		std::size_t sampleCount = static_cast<std::size_t>(totalFrames) * channels;
		floatSamples.resize(sampleCount);

		std::uint64_t framesRead = drwav_read_pcm_frames_f32(&wav, totalFrames, floatSamples.data());
		if (framesRead == 0)
		{
			drwav_uninit(&wav);
			return nullptr;
		}
		drwav_uninit(&wav);

		int inFrames = static_cast<int>(framesRead);
		if (inFrames <= 0) return nullptr;

		std::vector<float> processed;
		if (sampleRate != 48000)
			processed = ResampleCubic(floatSamples.data(), inFrames, channels, sampleRate, 48000);
		else
			processed = std::move(floatSamples);

		std::vector<std::uint8_t> finalData;
		ConvertFloatToInt16(processed.data(), static_cast<int>(processed.size()), finalData);

		auto asset = std::make_shared<StaticSoundAsset>();
		asset->channels = channels;
		asset->sampleRate = 48000;
		asset->format = SampleFormat::Int16;
		asset->data = std::move(finalData);

		return asset;
	}
};

std::unique_ptr<IAssetLoader> CreateWavStaticLoader()
{
	return std::make_unique<WavStaticLoader>();
}