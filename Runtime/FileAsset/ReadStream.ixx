module;

#include <cstdio>

export module FileAsset:ReadStream;

import std;
import Core.BasicTypes;
import Contract.Asset.Shared;

class ReadStream : public IReadStream
{
public:
	~ReadStream() override;
	explicit ReadStream(const std::filesystem::path& path);

	std::unique_ptr<IReadStream> Clone() const override;

	std::size_t Read(std::span<Core::Byte> buffer) override;
	bool Seek(std::size_t pos) noexcept override;
	std::size_t Tell() const noexcept override;
	std::size_t Size() const noexcept override;

	bool IsOpen() const noexcept override;
	bool Eof() const noexcept override;

private:
	std::filesystem::path m_filePath;
	FILE* m_file{ nullptr };
	std::size_t m_size{ 0 };
};