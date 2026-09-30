export module Runtime.Audio:StreamSoundBuffer;

import std;
import Core.TypeHierarchy;
import Client.Audio.Interfaces;
import Client.Asset.Contract;

export class StreamSoundBuffer : public ISoundBuffer
{
public:
	StreamSoundBuffer() = default;
	~StreamSoundBuffer() override = default;

	bool LoadFromAsset(std::shared_ptr<AssetData> asset) override
	{
		auto streamAsset = Core::Cast<StreamSoundAsset>(asset);
		if (!streamAsset)
			return false;

		m_fileStream = streamAsset->stream;
		return true;
	}

	IReadStream* GetStream() noexcept
	{
		return m_fileStream.get();
	}

private:
	std::shared_ptr<IReadStream> m_fileStream;
};