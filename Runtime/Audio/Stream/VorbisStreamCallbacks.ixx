module;

#include "ogg/ogg.h"
#include "vorbis/vorbisfile.h"

export module Runtime.Audio:VorbisDecoder;

import std;
import Client.Asset.Interfaces;

namespace Vorbis
{
	export size_t ReadFunc(void* ptr, size_t size, size_t nmemb, void* datasource)
	{
		auto* stream = static_cast<IReadStream*>(datasource);
		size_t bytesRequested = size * nmemb;

		std::span<std::byte> buffer(reinterpret_cast<std::byte*>(ptr), bytesRequested);
		size_t bytesRead = stream->Read(buffer);

		return bytesRead / size;
	}

	export int SeekFunc(void* datasource, ogg_int64_t offset, int whence)
	{
		auto* stream = static_cast<IReadStream*>(datasource);

		int64_t pos = 0;
		switch (whence)
		{
		case SEEK_SET:
			pos = offset;
			break;
		case SEEK_CUR:
			pos = static_cast<int64_t>(stream->Tell()) + offset;
			break;
		case SEEK_END:
			pos = static_cast<int64_t>(stream->Size()) + offset;
			break;
		default:
			return -1;
		}
		if (pos < 0)
			return -1;

		return stream->Seek(static_cast<size_t>(pos)) ? 0 : -1;
	}

	export long TellFunc(void* datasource)
	{
		auto* stream = static_cast<IReadStream*>(datasource);
		return static_cast<long>(stream->Tell());
	}

	export int CloseFunc(void*)
	{
		return 0; // 스트림 소유권은 외부
	}

	export ov_callbacks CreateCallbacks()
	{
		ov_callbacks cb{};
		cb.read_func = ReadFunc;
		cb.seek_func = SeekFunc;
		cb.tell_func = TellFunc;
		cb.close_func = CloseFunc;

		return cb;
	}
}