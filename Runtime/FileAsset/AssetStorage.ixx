export module FileAsset:AssetStorage;

import std;
import :ReadStream;
import Core.AssetPath;
import Core.BasicTypes;
import Contract.Asset.Shared;

class AssetStorage : public IAssetStorage
{
public:
	~AssetStorage() override = default;
	AssetStorage() = delete;

	explicit AssetStorage(const std::filesystem::path& path);
	bool Write(const Core::AssetPath& filename, const Core::ByteBuffer& buffer) noexcept override;
	bool Read(const Core::AssetPath& filename, Core::ByteBuffer& outBuffer) const override;
	bool WriteText(const Core::AssetPath& filename, const std::string& text) noexcept override;
	bool ReadText(const Core::AssetPath& filename, std::string& outText) const noexcept override;

	std::vector<Core::AssetPath> FindFiles(std::string_view extension) const override;

	std::unique_ptr<IReadStream> CreateReadStream(const Core::AssetPath& filename) override;

private:
	std::filesystem::path m_path;
};