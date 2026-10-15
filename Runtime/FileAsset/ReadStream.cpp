module;

#include <cstdio>

module FileAsset;

ReadStream::~ReadStream()
{
	if (m_file)
		std::fclose(m_file);
}

ReadStream::ReadStream(const std::filesystem::path& path)
	: m_filePath{ path }
{
#ifdef _WIN32
	FILE* file = nullptr;
	_wfopen_s(&file, path.c_str(), L"rb");
	m_file = file;
#else
	m_file = std::fopen(path.string().c_str(), "rb");
#endif

	if (!m_file)
		return;

	m_size = static_cast<std::size_t>(std::filesystem::file_size(path));
}

std::unique_ptr<IReadStream> ReadStream::Clone() const
{
	return std::make_unique<ReadStream>(m_filePath);
}

std::size_t ReadStream::Read(std::span<Core::Byte> buffer)
{
	if (!m_file)
		return 0;

	return std::fread(buffer.data(), 1, buffer.size(), m_file);
}

bool ReadStream::Seek(std::size_t pos) noexcept
{
	if (!m_file)
		return false;

#ifdef _WIN32
	return _fseeki64(m_file, static_cast<long long>(pos), SEEK_SET) == 0;
#else
	return fseeko(m_file, pos, SEEK_SET) == 0;
#endif
}

std::size_t ReadStream::Tell() const noexcept
{
	if (!m_file)
		return 0;

#ifdef _WIN32
	return static_cast<std::size_t>(_ftelli64(m_file));
#else
	return static_cast<std::size_t>(ftello(m_file));
#endif
}

std::size_t ReadStream::Size() const noexcept
{
	return m_size;
}

bool ReadStream::IsOpen() const noexcept
{
	return m_file != nullptr;
}

bool ReadStream::Eof() const noexcept
{
	return m_file ? std::feof(m_file) != 0 : true;
}