import std;
import Core.BasicTypes;
import Core.Logger;
import Contract.Common.Asset;
import Contract.Audio.Assets;
import Contract.Asset.Backend;
import FileAsset.VorbisDecoder;
import FileAsset.AudioProcessing;

class OggStaticLoader : public IAssetLoader
{
public:
	~OggStaticLoader() override = default;

	std::shared_ptr<Asset> Load(AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<MemoryInput&>(source);
		return LoadFromMemory(mem.buffer);
	}

private:
	std::shared_ptr<StaticSoundAsset> LoadFromMemory(const Core::ByteBuffer& buffer)
	{
		VorbisDecoder decoder;

		if (!decoder.Open(buffer.data(), buffer.size()))
		{
			Core::Logger::Error("OggStaticLoader: Ogg Vorbis 파일을 열지 못함 ({}바이트)", buffer.size());
			return nullptr;
		}

		const vorbis_info* info = decoder.GetInfo();
		if (!info || info->channels <= 0 || info->rate <= 0)
		{
			Core::Logger::Error("OggStaticLoader: 채널 수나 레이트를 알 수 없음");
			return nullptr;
		}

		const int srcRate = static_cast<int>(info->rate);
		int totalFrames = 0;
		int channels = 0;

		std::vector<float> pcm = decoder.DecodeAll(totalFrames, channels);
		if (pcm.empty() || totalFrames <= 0)
		{
			Core::Logger::Error("OggStaticLoader: 디코딩한 소리가 없음");
			return nullptr;
		}

		return MakeStaticSound(std::move(pcm), totalFrames, channels, srcRate);
	}
};

std::unique_ptr<IAssetLoader> CreateOggStaticLoader()
{
	return std::make_unique<OggStaticLoader>();
}
