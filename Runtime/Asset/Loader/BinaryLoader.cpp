import std;
import Core.Types;
import Client.Asset.Contract;

class BinaryLoader : public IAssetLoader
{
public:
	~BinaryLoader() override = default;

	std::shared_ptr<AssetData> Load(AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<MemoryInput&>(source);
		return LoadFromMemory(std::move(mem.buffer));
	}

private:
	std::shared_ptr<BinaryAsset> LoadFromMemory(Core::ByteBuffer buffer)
	{
		auto asset = std::make_shared<BinaryAsset>();
		asset->buffer = std::move(buffer);

		return asset;
	}
};

std::unique_ptr<IAssetLoader> CreateBinaryLoader()
{
	return std::make_unique<BinaryLoader>();
}