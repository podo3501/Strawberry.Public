export module Contract.Audio.Assets:AudioTypes;

import std;

export enum class SoundType : int
{
	Static,
	Stream,
	Count
};

export enum class AudioGroup : int
{
	BGM,
	SFX,
	UI,
	System,
	Count
};
