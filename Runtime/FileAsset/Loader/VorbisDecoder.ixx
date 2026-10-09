module;

#ifdef _DEBUG
#pragma comment(lib, "ogg_Debug.lib")
#pragma comment(lib, "vorbis_Debug.lib")
#pragma comment(lib, "vorbisfile_Debug.lib")
#else
#pragma comment(lib, "ogg_Release.lib")
#pragma comment(lib, "vorbis_Release.lib")
#pragma comment(lib, "vorbisfile_Release.lib")
#endif

#include "ogg/ogg.h"
#include "vorbis/vorbisfile.h"

export module FileAsset.VorbisDecoder;

import std;

export class VorbisDecoder
{
public:
	VorbisDecoder() = default;

	~VorbisDecoder()
	{
		Close();
	}

	bool Open(const void* data, size_t size)
	{
		Close();

		ctx.data = static_cast<const unsigned char*>(data);
		ctx.size = size;
		ctx.offset = 0;

		cb.read_func = ReadCB;
		cb.seek_func = SeekCB;
		cb.tell_func = TellCB;
		cb.close_func = CloseCB;

		if (ov_open_callbacks(&ctx, &vf, nullptr, 0, cb) < 0)
			return false;

		opened = true;
		return true;
	}

	std::vector<float> DecodeAll(int& outFrames, int& outChannels)
	{
		outFrames = 0;
		outChannels = 0;

		const vorbis_info* info = GetInfo();
		if (!info) return {};

		outChannels = info->channels;

		std::vector<float> temp;
		int currentSection = 0;

		while (true)
		{
			float** pcm = nullptr;

			long samples = ov_read_float(
				&vf,
				&pcm,
				1024,
				&currentSection);

			if (samples <= 0)
				break;

			size_t oldSize = temp.size();
			size_t sampleCount = static_cast<size_t>(samples) * outChannels;

			temp.resize(oldSize + sampleCount);

			for (int ch = 0; ch < outChannels; ch++)
			{
				for (int i = 0; i < samples; i++)
				{
					temp[oldSize + i * outChannels + ch] = pcm[ch][i];
				}
			}

			outFrames += static_cast<int>(samples);
		}

		return temp;
	}

	void Close()
	{
		if (opened)
		{
			ov_clear(&vf);
			opened = false;
		}
	}

	bool IsOpened() const
	{
		return opened;
	}

	const vorbis_info* GetInfo() const
	{
		if (!opened) return nullptr;
		return ov_info(const_cast<OggVorbis_File*>(&vf), -1);
	}

private:
	// 콜백 함수 정의 (C-style API 바인딩)
	static size_t ReadCB(void* ptr, size_t size, size_t nmemb, void* datasource)
	{
		if (size == 0) return 0;

		auto* ctx = static_cast<MemoryContext*>(datasource);

		size_t bytesRequested = size * nmemb;
		size_t remaining = ctx->size - ctx->offset;

		size_t bytesToRead = (bytesRequested < remaining) ? bytesRequested : remaining;
		if (bytesToRead == 0) return 0;

		std::memcpy(ptr, ctx->data + ctx->offset, bytesToRead);
		ctx->offset += bytesToRead;

		return bytesToRead / size;
	}

	static int SeekCB(void* datasource, ogg_int64_t offset, int whence)
	{
		auto* ctx = static_cast<MemoryContext*>(datasource);

		int64_t pos = 0;
		switch (whence)
		{
		case SEEK_SET: pos = offset; break;
		case SEEK_CUR: pos = static_cast<int64_t>(ctx->offset) + offset; break;
		case SEEK_END: pos = static_cast<int64_t>(ctx->size) + offset; break;
		default: return -1;
		}
		if (pos < 0 || pos > static_cast<int64_t>(ctx->size)) return -1;

		ctx->offset = static_cast<size_t>(pos);
		return 0;
	}

	static long TellCB(void* datasource)
	{
		auto* ctx = static_cast<MemoryContext*>(datasource);
		return static_cast<long>(ctx->offset);
	}

	static int CloseCB(void*)
	{
		return 0;
	}

private:
	OggVorbis_File vf{};
	ov_callbacks cb{};
	bool opened = false;

	struct MemoryContext
	{
		const unsigned char* data = nullptr;
		size_t size = 0;
		size_t offset = 0;
	} ctx;
};