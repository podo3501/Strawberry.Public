export module FileAsset:AssetStorage;

import std;
import :ReadStream;
import Core.Types;
import Contract.Asset;

class AssetStorage : public IAssetStorage
{
public:
	~AssetStorage() override = default;
	AssetStorage() = delete;

	explicit AssetStorage(const std::filesystem::path& path);
	bool Write(const std::filesystem::path& filename, const Core::ByteBuffer& buffer) noexcept override;
	bool Read(const std::filesystem::path& filename, Core::ByteBuffer& outBuffer) const override;
	bool WriteText(const std::filesystem::path& filename, const std::string& text) noexcept override;
	bool ReadText(const std::filesystem::path& filename, std::string& outText) const noexcept override;

	std::filesystem::path MakeAssetFilePath(const std::filesystem::path& filename) const noexcept override;
	std::unique_ptr<IReadStream> CreateReadStream(const std::filesystem::path& filename) override;

private:
	std::filesystem::path m_path;
};