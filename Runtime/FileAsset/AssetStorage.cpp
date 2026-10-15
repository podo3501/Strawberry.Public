module FileAsset;

import Core.Utils;

AssetStorage::AssetStorage(const std::filesystem::path& path)
	: m_path{ path }
{}

bool AssetStorage::Write(const std::filesystem::path& filename, const Core::ByteBuffer& buffer) noexcept
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

bool AssetStorage::Read(const std::filesystem::path& filename, Core::ByteBuffer& outBuffer) const
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

bool AssetStorage::WriteText(const std::filesystem::path& filename, const std::string& text) noexcept
{
	Core::ByteBuffer buffer;
	buffer.resize(text.size());
	if (!text.empty())
		std::memcpy(buffer.data(), text.data(), text.size());

	return Write(filename, buffer);
}

bool AssetStorage::ReadText(const std::filesystem::path& filename, std::string& outText) const noexcept

{
	Core::ByteBuffer buffer;
	if (!Read(filename, buffer))
		return false;

	outText.assign(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	return true;
}

std::filesystem::path AssetStorage::MakeAssetFilePath(const std::filesystem::path& filename) const noexcept
{
	return Core::ResolvePath(m_path, filename);
}

std::unique_ptr<IReadStream> AssetStorage::	CreateReadStream(const std::filesystem::path& filename)
{
	auto stream = std::make_unique<ReadStream>(Core::ResolvePath(m_path, filename));
	if (!stream->IsOpen())
		return nullptr;

	return stream;
}

std::unique_ptr<IAssetStorage> CreateAssetStorage(const std::filesystem::path& path)
{
	return std::make_unique<AssetStorage>(path);
}