export module Service.Audio:Voice;

import std;
import Contract.Audio.Assets;
import Contract.Audio.Service;
import Contract.Audio.Backend;

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