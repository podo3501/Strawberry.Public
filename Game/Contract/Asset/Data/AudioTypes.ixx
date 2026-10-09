export module Contract.Asset.Data:AudioTypes;

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
