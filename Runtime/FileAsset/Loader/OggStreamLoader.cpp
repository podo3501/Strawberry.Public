import std;
import Contract.Common.Asset;
import Contract.Audio.Assets;
import Contract.Asset.Backend;
import Contract.Asset.Shared;

class OggStreamLoader : public IAssetLoader
{
public:
	~OggStreamLoader() override = default;

	bool PreferStream() const override { return true; }

	std::shared_ptr<Asset> Load(AssetInput& source) override
	{
		if (!source.IsStream()) return nullptr;

		auto& streamInput = static_cast<StreamInput&>(source);
		return LoadFromStream(std::move(streamInput.stream));
	}

private:
	std::shared_ptr<StreamSoundAsset> LoadFromStream(std::unique_ptr<IReadStream> stream)
	{
		auto asset = std::make_shared<StreamSoundAsset>();
		asset->stream = std::move(stream);

		return asset;
	}
};

std::unique_ptr<IAssetLoader> CreateOggStreamLoader()
{
	return std::make_unique<OggStreamLoader>();
}