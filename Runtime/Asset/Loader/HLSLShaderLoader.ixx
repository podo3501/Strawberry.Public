export module Runtime.AssetLoader:HLSLShader;

import std;
import Client.Asset.Contract;

export class HLSLShaderLoader : public Client::IAssetLoader
{
public:
	~HLSLShaderLoader() override = default;

	std::shared_ptr <Client::AssetData > Load(Client::AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<Client::MemoryInput&>(source);
		return LoadFromMemory(std::move(mem.buffer));
	}

private:
	std::shared_ptr<Client::ShaderAsset> LoadFromMemory(std::vector<std::byte> buffer)
	{
		auto asset = std::make_shared<Client::ShaderAsset>();

		std::string hlsl(
			reinterpret_cast<const char*>(buffer.data()),
			buffer.size()
		);

		asset->hlslSource = std::move(hlsl);
		return asset;
	}
};

std::unique_ptr<Client::IAssetLoader> Client::CreateHLSLShaderLoader()
{
	return std::make_unique<HLSLShaderLoader>();
}