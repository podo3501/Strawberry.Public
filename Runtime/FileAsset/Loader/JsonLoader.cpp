module;

#include <nlohmann/json.hpp>

export module FileAsset.JsonLoader;

import std;
import Core.BasicTypes;
import Contract.Asset.Backend;
import Contract.Common.Asset;
import Runtime.Serialization;

template<typename T>
class JsonLoader : public IAssetLoader
{
public:
	~JsonLoader() override = default;

	std::shared_ptr<Asset> Load(AssetInput& source) override
	{
		if (source.IsStream()) return nullptr;

		auto& mem = static_cast<MemoryInput&>(source);
		return LoadFromMemory(std::move(mem.buffer));
	}

private:
	std::shared_ptr<Asset> LoadFromMemory(Core::ByteBuffer buffer)
	{
		auto asset = std::make_shared<T>();

		nlohmann::json rData = nlohmann::json::parse(buffer.begin(), buffer.end());
		DeserializeClass(rData, *asset);

		return asset;
	}
};

export template<typename T>
std::unique_ptr<IAssetLoader> CreateJsonLoader()
{
	return std::make_unique<JsonLoader<T>>();
}