module;

#include <cstdio>

module FileAsset;

import Core.Logger;
import Core.Utils;

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

	// 이미 연 파일 핸들로 크기를 구한다 (열자마자 파일이 사라져도 예외가 나지 않는다)
#ifdef _WIN32
	_fseeki64(m_file, 0, SEEK_END);
	const long long size = _ftelli64(m_file);
	_fseeki64(m_file, 0, SEEK_SET);
#else
	fseeko(m_file, 0, SEEK_END);
	const long long size = ftello(m_file);
	fseeko(m_file, 0, SEEK_SET);
#endif

	if (size < 0)
	{
		std::fclose(m_file);
		m_file = nullptr;
		return;
	}

	m_size = static_cast<std::size_t>(size);
}

std::unique_ptr<IReadStream> ReadStream::Clone() const
{
	auto clone = std::make_unique<ReadStream>(m_filePath);
	if (!clone->IsOpen())
	{
		Core::Logger::Error("ReadStream: 스트림을 복제하지 못함 ({})", Core::WStringToString(m_filePath.wstring()));
		return nullptr;
	}

	return clone;
}

std::size_t ReadStream::Read(std::span<Core::Byte> buffer)
{
	if (!m_file)
		return 0;

	return std::fread(buffer.data(), 1, buffer.size(), m_file);
}

bool ReadStream::Seek(std::size_t pos) noexcept
{
	if (!m_file || pos > m_size)
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
