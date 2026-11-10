#pragma warning(push)
#pragma warning(disable: 4244)
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#pragma warning(pop)

import std;
import Core.BasicTypes;
import Core.Logger;
import Contract.Common.Asset;
import Contract.Audio.Assets;
import Contract.Asset.Backend;
import FileAsset.AudioProcessing;

// RIFF 청크를 훑어 data 청크가 파일 끝을 넘는지 본다. dr_wav는 짧은 파일을 조용히 있는 만큼만 읽기 때문에 따로 확인해야 한다
static bool DataChunkFitsInFile(const Core::ByteBuffer& buffer)
{
	constexpr std::size_t kRiffHeaderSize = 12;      // "RIFF" + 크기 + "WAVE"
	constexpr std::uint32_t kUnknownSize = 0xFFFFFFFF; // 스트리밍으로 쓴 WAV는 크기를 모른다고 적는다

	std::size_t pos = kRiffHeaderSize;
	while (pos + 8 <= buffer.size())
	{
		std::uint32_t size = 0;
		std::memcpy(&size, buffer.data() + pos + 4, sizeof(size));
		const std::size_t body = pos + 8;
		const bool isData = std::memcmp(buffer.data() + pos, "data", 4) == 0;

		if (size == kUnknownSize)
			return isData;
		if (size > buffer.size() - body)
			return false;   // 어느 청크든 파일 끝을 넘으면 잘린 파일이다
		if (isData)
			return true;

		pos = body + size + (size & 1);   // 청크는 짝수 바이트로 맞춰진다
	}
	return true;   // data 청크를 못 찾으면 dr_wav의 판단에 맡긴다
}

class WavStaticLoader : public IAssetLoader
{
public:
	~WavStaticLoader() override = default;

	std::shared_ptr<Asset> Load(AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<MemoryInput&>(source);
		return LoadFromMemory(mem.buffer);
	}

private:
	std::shared_ptr<StaticSoundAsset> LoadFromMemory(const Core::ByteBuffer& buffer)
	{
		drwav wav;

		if (!drwav_init_memory(&wav, buffer.data(), buffer.size(), nullptr))
		{
			Core::Logger::Error("WavStaticLoader: WAV 파일이 아님 ({}바이트)", buffer.size());
			return nullptr;
		}

		const std::uint32_t channels = wav.channels;
		const std::uint32_t sampleRate = wav.sampleRate;
		const std::uint64_t totalFrames = wav.totalPCMFrameCount;

		if (channels == 0 || sampleRate == 0 || totalFrames == 0)
		{
			Core::Logger::Error("WavStaticLoader: 소리가 없는 WAV (채널 {}, 레이트 {}, 프레임 {})", channels, sampleRate, totalFrames);
			drwav_uninit(&wav);
			return nullptr;
		}

		if (!DataChunkFitsInFile(buffer) || totalFrames > std::numeric_limits<int>::max() / channels)
		{
			Core::Logger::Error("WavStaticLoader: 헤더의 데이터 크기가 파일보다 큼 (프레임 {}, 파일 {}바이트)", totalFrames, buffer.size());
			drwav_uninit(&wav);
			return nullptr;
		}

		std::vector<float> samples(static_cast<std::size_t>(totalFrames) * channels);
		const std::uint64_t framesRead = drwav_read_pcm_frames_f32(&wav, totalFrames, samples.data());
		drwav_uninit(&wav);

		if (framesRead != totalFrames)
		{
			Core::Logger::Error("WavStaticLoader: 데이터가 헤더보다 짧음 ({} / {} 프레임)", framesRead, totalFrames);
			return nullptr;
		}

		return MakeStaticSound(std::move(samples), static_cast<int>(totalFrames), static_cast<int>(channels), static_cast<int>(sampleRate));
	}
};

std::unique_ptr<IAssetLoader> CreateWavStaticLoader()
{
	return std::make_unique<WavStaticLoader>();
}
