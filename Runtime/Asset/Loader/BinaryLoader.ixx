export module Runtime.AssetLoader:Binary;

import std;
import Core.Types;
import Client.Asset.Contract;

export class BinaryLoader : public Client::IAssetLoader
{
public:
	~BinaryLoader() override = default;

	std::shared_ptr<Client::AssetData> Load(Client::AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<Client::MemoryInput&>(source);
		return LoadFromMemory(std::move(mem.buffer));
	}

private:
	std::shared_ptr<Client::BinaryAsset> LoadFromMemory(Core::ByteBuffer buffer)
	{
		auto asset = std::make_shared<Client::BinaryAsset>();
		asset->buffer = std::move(buffer);

		return asset;
	}
};

std::unique_ptr<Client::IAssetLoader> Client::CreateBinaryLoader()
{
	return std::make_unique<BinaryLoader>();
}