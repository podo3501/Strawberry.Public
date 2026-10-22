export module Contract.Audio.Shared:PlaybackState;

export enum class PlaybackState : int
{
	Pending,
	Playing,
	Paused,
	Stopped,
	Count
};