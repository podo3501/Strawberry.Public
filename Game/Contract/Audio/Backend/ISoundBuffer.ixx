export module Contract.Audio.Backend:ISoundBuffer;

import std;
import Contract.Common.Asset;

export struct ISoundBuffer
{
	virtual ~ISoundBuffer() = default;
	virtual bool LoadFromAsset(std::shared_ptr<Asset> asset) = 0;
};