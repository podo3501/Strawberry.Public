export module Runtime.Asset:AssetStorage;

import std;
import :ReadStream;
import Core.Types;
import Core.Utils;
import Client.Asset.Contract;

class AssetStorage : public IAssetStorage
{
public:
	~AssetStorage() override = default;
	AssetStorage() = delete;

	explicit AssetStorage(const std::filesystem::path& path)
		: m_path{ path }
	{
	}

	bool Write(const std::filesystem::path& filename, const Core::ByteBuffer& buffer) noexcept override
	{
		auto absolutePath = Core::ResolvePath(m_path, filename);
		std::ofstream file(absolutePath, std::ios::binary | std::ios::trunc);
		if (!file)
			return false;

		if (!buffer.empty())
		{
			file.write(
				reinterpret_cast<const char*>(buffer.data()),
				static_cast<std::streamsize>(buffer.size()));

			if (!file)
				return false;
		}

		return true;
	}

	bool Read(const std::filesystem::path& filename, Core::ByteBuffer& outBuffer) const override
	{
		outBuffer.clear();

		auto absolutePath = Core::ResolvePath(m_path, filename);
		std::ifstream file(absolutePath, std::ios::binary | std::ios::ate);
		if (!file)
			return false;

		const std::streamsize size = file.tellg();
		if (size < 0)
			return false;

		if (size == 0)
			return true;

		file.seekg(0, std::ios::beg);

		outBuffer.resize(static_cast<std::size_t>(size));

		if (!file.read(reinterpret_cast<char*>(outBuffer.data()), size))
		{
			outBuffer.clear();
			return false;
		}

		return true;
	}

	bool WriteText(const std::filesystem::path& filename, const std::string& text) noexcept override
	{
		Core::ByteBuffer buffer;
		buffer.resize(text.size());
		if (!text.empty())
			std::memcpy(buffer.data(), text.data(), text.size());

		return Write(filename, buffer);
	}

	bool ReadText(const std::filesystem::path& filename, std::string& outText) const noexcept override
	{
		Core::ByteBuffer buffer;
		if (!Read(filename, buffer))
			return false;

		outText.assign(reinterpret_cast<const char*>(buffer.data()), buffer.size());
		return true;
	}

	std::filesystem::path MakeAssetFilePath(const std::filesystem::path& filename) const noexcept override
	{
		return Core::ResolvePath(m_path, filename);
	}

	std::unique_ptr<IReadStream> CreateReadStream(const std::filesystem::path& filename) override
	{
		auto stream = std::make_unique<ReadStream>(Core::ResolvePath(m_path, filename));
		if (!stream->IsOpen())
			return nullptr;

		return stream;
	}

private:
	std::filesystem::path m_path;
};

std::unique_ptr<IAssetStorage> CreateAssetStorage(const std::filesystem::path& path)
{
	return std::make_unique<AssetStorage>(path);
}