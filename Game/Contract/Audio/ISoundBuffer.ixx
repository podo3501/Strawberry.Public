export module Client.Audio.Interfaces:ISoundBuffer;

import std;
import Client.Asset.AssetData;

export struct ISoundBuffer
{
	virtual ~ISoundBuffer() = default;
	virtual bool LoadFromAsset(std::shared_ptr<AssetData> asset) = 0;
};