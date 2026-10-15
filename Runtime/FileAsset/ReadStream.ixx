module;

#include <cstdio>

export module FileAsset:ReadStream;

import std;
import Core.Types;
import Contract.Asset;

class ReadStream : public IReadStream
{
public:
	~ReadStream() override;
	explicit ReadStream(const std::filesystem::path& path);
	std::unique_ptr<IReadStream> Clone() const;

	std::size_t Read(std::span<Core::Byte> buffer);
	bool Seek(std::size_t pos) noexcept;
	std::size_t Tell() const noexcept;
	std::size_t Size() const noexcept;

	bool IsOpen() const noexcept;
	bool Eof() const noexcept;

private:
	std::filesystem::path m_filePath;
	FILE* m_file{ nullptr };
	std::size_t m_size{ 0 };
};