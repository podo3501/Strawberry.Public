export module SDLAudio:StreamSoundBuffer;

import std;
import Contract.Audio.Backend;
import Contract.Asset.Shared;
import Contract.Common.Asset;

export class StreamSoundBuffer : public ISoundBuffer
{
public:
	StreamSoundBuffer() = default;
	~StreamSoundBuffer() override = default;

	virtual bool LoadFromAsset(std::shared_ptr<Asset> asset) override;
	IReadStream* GetStream() noexcept;

private:
	std::shared_ptr<IReadStream> m_fileStream;
};