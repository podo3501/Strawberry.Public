import std;
import Core.Types;
import Client.Asset.Contract;

class HLSLShaderLoader : public IAssetLoader
{
public:
	~HLSLShaderLoader() override = default;

	std::shared_ptr <AssetData > Load(AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<MemoryInput&>(source);
		return LoadFromMemory(std::move(mem.buffer));
	}

private:
	std::shared_ptr<ShaderAsset> LoadFromMemory(Core::ByteBuffer buffer)
	{
		auto asset = std::make_shared<ShaderAsset>();

		std::string hlsl(
			reinterpret_cast<const char*>(buffer.data()),
			buffer.size()
		);

		asset->hlslSource = std::move(hlsl);
		return asset;
	}
};

std::unique_ptr<IAssetLoader> CreateHLSLShaderLoader()
{
	return std::make_unique<HLSLShaderLoader>();
}