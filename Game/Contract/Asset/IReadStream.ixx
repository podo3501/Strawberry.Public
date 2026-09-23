export module Client.Asset.Contract:IReadStream;

import std;
import Core.Types;

namespace Client
{
	export struct IReadStream
	{
		virtual ~IReadStream() = default;

		virtual std::unique_ptr<IReadStream> Clone() const = 0;
		virtual std::size_t Read(std::span<Core::Byte> buffer) = 0;
		virtual bool Seek(std::size_t pos) noexcept = 0;
		virtual std::size_t Tell() const noexcept = 0;
		virtual std::size_t Size() const noexcept = 0;
		virtual bool IsOpen() const noexcept = 0;
		virtual bool Eof() const noexcept = 0;
	};
}