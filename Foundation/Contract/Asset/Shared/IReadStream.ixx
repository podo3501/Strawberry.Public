export module Contract.Asset.Shared:IReadStream;

import std;
import Core.BasicTypes;

export struct IReadStream
{
	virtual ~IReadStream() = default;

	// 같은 소스를 처음 위치에서 다시 연다. 열지 못하면 nullptr
	virtual std::unique_ptr<IReadStream> Clone() const = 0;
	virtual std::size_t Read(std::span<Core::Byte> buffer) = 0;
	// Size()를 넘는 위치로는 옮길 수 없고 false를 돌려준다
	virtual bool Seek(std::size_t pos) noexcept = 0;
	virtual std::size_t Tell() const noexcept = 0;
	virtual std::size_t Size() const noexcept = 0;
	virtual bool IsOpen() const noexcept = 0;
};