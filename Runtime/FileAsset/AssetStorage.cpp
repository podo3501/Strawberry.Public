module FileAsset;

import Core.AssetPath;
import Core.Utils;

AssetStorage::AssetStorage(const std::filesystem::path& path)
	: m_path{ path }
{}

bool AssetStorage::Write(const Core::AssetPath& filename, const Core::ByteBuffer& buffer) noexcept
{
	auto absolutePath = Core::ResolvePath(m_path, filename.ToPath());
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

bool AssetStorage::Read(const Core::AssetPath& filename, Core::ByteBuffer& outBuffer) const
{
	outBuffer.clear();

	auto absolutePath = Core::ResolvePath(m_path, filename.ToPath());
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

bool AssetStorage::WriteText(const Core::AssetPath& filename, const std::string& text) noexcept
{
	Core::ByteBuffer buffer;
	buffer.resize(text.size());
	if (!text.empty())
		std::memcpy(buffer.data(), text.data(), text.size());

	return Write(filename, buffer);
}

bool AssetStorage::ReadText(const Core::AssetPath& filename, std::string& outText) const noexcept

{
	Core::ByteBuffer buffer;
	if (!Read(filename, buffer))
		return false;

	outText.assign(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	return true;
}

std::vector<Core::AssetPath> AssetStorage::FindFiles(std::string_view extension) const
{
	std::vector<Core::AssetPath> found;

	std::error_code error;
	std::filesystem::recursive_directory_iterator it(m_path, error);
	if (error)
		return found;   // 루트가 없거나 읽을 수 없음

	for (const std::filesystem::recursive_directory_iterator end; it != end; it.increment(error))
	{
		if (error)
			break;
		if (!it->is_regular_file(error) || it->path().extension().string() != extension)
			continue;

		if (const auto path = Core::AssetPath::TryMake(std::filesystem::relative(it->path(), m_path).generic_string()))
			found.push_back(*path);
	}

	return found;
}

std::unique_ptr<IReadStream> AssetStorage::	CreateReadStream(const Core::AssetPath& filename)
{
	auto stream = std::make_unique<ReadStream>(Core::ResolvePath(m_path, filename.ToPath()));
	if (!stream->IsOpen())
		return nullptr;

	return stream;
}

std::unique_ptr<IAssetStorage> CreateAssetStorage(const std::filesystem::path& path)
{
	return std::make_unique<AssetStorage>(path);
}