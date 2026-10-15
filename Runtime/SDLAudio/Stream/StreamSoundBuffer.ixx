export module SDLAudio:StreamSoundBuffer;

import std;
import Contract.Audio.Interfaces;
import Contract.Asset.Interfaces;
import Contract.Asset.AssetData;

export class StreamSoundBuffer : public ISoundBuffer
{
public:
	StreamSoundBuffer() = default;
	~StreamSoundBuffer() override = default;

	virtual bool LoadFromAsset(std::shared_ptr<AssetData> asset) override;
	IReadStream* GetStream() noexcept;

private:
	std::shared_ptr<IReadStream> m_fileStream;
};