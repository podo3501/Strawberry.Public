export module Contract.Audio.Interfaces:ISoundBuffer;

import std;
import Contract.Asset.AssetData;

export struct ISoundBuffer
{
	virtual ~ISoundBuffer() = default;
	virtual bool LoadFromAsset(std::shared_ptr<AssetData> asset) = 0;
};