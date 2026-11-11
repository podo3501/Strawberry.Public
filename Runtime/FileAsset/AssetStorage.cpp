module FileAsset;

import Core.AssetPath;
import Core.Utils;
import Core.Logger;

AssetStorage::AssetStorage(const std::filesystem::path& path)
	: m_path{ path }
{}

namespace
{
	std::string ToLog(const std::filesystem::path& path)
	{
		return Core::WStringToUTF8(path.wstring());
	}
}

bool AssetStorage::Write(const Core::AssetPath& filename, const Core::ByteBuffer& buffer) noexcept
{
	const auto absolutePath = Core::ResolvePath(m_path, filename.ToPath());

	std::error_code error;
	std::filesystem::create_directories(absolutePath.parent_path(), error);
	if (error)
	{
		Core::Logger::Error("AssetStorage: 폴더를 만들지 못함 ({}): {}", ToLog(absolutePath.parent_path()), error.message());
		return false;
	}

	// 임시 파일에 다 쓴 뒤 이름을 바꿔 덮어쓴다. 도중에 실패해도 기존 파일이 그대로 남는다
	auto tempPath = absolutePath;
	tempPath += ".tmp";

	{
		std::ofstream file(tempPath, std::ios::binary | std::ios::trunc);
		if (!file)
		{
			Core::Logger::Error("AssetStorage: 파일을 열지 못함 ({})", ToLog(tempPath));
			return false;
		}

		if (!buffer.empty())
			file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));

		file.close();
		if (!file)
		{
			Core::Logger::Error("AssetStorage: 파일에 쓰지 못함 ({})", ToLog(tempPath));
			std::filesystem::remove(tempPath, error);
			return false;
		}
	}

	std::filesystem::rename(tempPath, absolutePath, error);
	if (error)
	{
		Core::Logger::Error("AssetStorage: 파일 이름을 바꾸지 못함 ({}): {}", ToLog(absolutePath), error.message());
		std::filesystem::remove(tempPath, error);
		return false;
	}

	return true;
}

bool AssetStorage::Read(const Core::AssetPath& filename, Core::ByteBuffer& outBuffer) const
{
	outBuffer.clear();

	const auto absolutePath = Core::ResolvePath(m_path, filename.ToPath());
	std::ifstream file(absolutePath, std::ios::binary | std::ios::ate);
	if (!file)
	{
		std::error_code error;
		if (std::filesystem::exists(absolutePath, error))   // 없는 파일은 호출하는 쪽이 판단한다. 있는데 못 여는 경우만 남긴다
			Core::Logger::Error("AssetStorage: 파일을 열지 못함 ({})", ToLog(absolutePath));
		return false;
	}

	const std::streamsize size = file.tellg();
	if (size < 0)
	{
		Core::Logger::Error("AssetStorage: 파일 크기를 알 수 없음 ({})", ToLog(absolutePath));
		return false;
	}

	if (size == 0)
		return true;

	file.seekg(0, std::ios::beg);

	outBuffer.resize(static_cast<std::size_t>(size));

	if (!file.read(reinterpret_cast<char*>(outBuffer.data()), size))
	{
		outBuffer.clear();
		Core::Logger::Error("AssetStorage: 파일을 끝까지 읽지 못함 ({})", ToLog(absolutePath));
		return false;
	}

	return true;
}

bool AssetStorage::WriteText(const Core::AssetPath& filename, const std::string& text) noexcept
{
	Core::ByteBuffer buffer(text.size());
	if (!text.empty())
		std::memcpy(buffer.data(), text.data(), text.size());

	return Write(filename, buffer);
}

bool AssetStorage::ReadText(const Core::AssetPath& filename, std::string& outText) const noexcept
{
	outText.clear();

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
		if (!it->is_regular_file(error) || Core::PathToUtf8(it->path().extension()) != extension)
			continue;

		const auto relative = std::filesystem::relative(it->path(), m_path, error);
		if (error)
			continue;

		if (const auto path = Core::AssetPath::TryMake(Core::PathToUtf8(relative)))
			found.push_back(*path);
	}

	return found;
}

std::optional<std::filesystem::file_time_type> AssetStorage::GetModifiedTime(const Core::AssetPath& filename) const noexcept
{
	std::error_code error;
	const auto time = std::filesystem::last_write_time(Core::ResolvePath(m_path, filename.ToPath()), error);
	if (error)
		return std::nullopt;   // 파일이 없거나 읽을 수 없음
	return time;
}

std::unique_ptr<IReadStream> AssetStorage::CreateReadStream(const Core::AssetPath& filename)
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