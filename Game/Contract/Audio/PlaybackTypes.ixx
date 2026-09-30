export module Client.Audio.Interfaces:Playback;

export struct PlaybackParams
{
	float volume{ 1.0f };
	bool loop{ false };
};

export enum class PlaybackState : int
{
	Pending,
	Playing,
	Paused,
	Stopped,
	Count
};
