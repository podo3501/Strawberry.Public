export module Client.Audio:Voice;

import std;
import :VoiceHandle;
import :SoundHandle;
import Client.Asset.Data;
import Client.Audio.Interfaces;

export struct Voice
{
	VoiceHandle voiceHandle{};
	SoundHandle soundHandle{};
	ISoundInstance* instance{ nullptr };
	SoundDesc desc{};

	std::uint32_t playbackTime{ 0 };

	void Reset() noexcept
	{
		voiceHandle = {};
		soundHandle = {};
		instance = nullptr;
		desc = {};
		playbackTime = 0;
	}

	bool StopAndReset() noexcept
	{
		if (!instance)
			return true;

		instance->Stop();
		Reset();

		return true;
	}
};