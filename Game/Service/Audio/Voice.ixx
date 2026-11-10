export module Service.Audio:Voice;

import std;
import Contract.Audio.Assets;
import Contract.Audio.Service;
import Contract.Audio.Backend;

export struct Voice
{
	VoiceHandle voiceHandle{};
	SoundHandle soundHandle{};
	ISoundInstance* instance{ nullptr }; // 재생을 시작하기 전(대기 중)에는 없다.
	const SoundDesc* desc{ nullptr };    // 소리 테이블의 설명. 테이블은 서비스가 붙들고 있다.
	float volume{ 1.0f }; // 소리 자체의 볼륨. 마스터와 그룹 볼륨을 곱하기 전의 값이다.

	std::uint32_t playbackTime{ 0 };

	void Reset() noexcept
	{
		voiceHandle = {};
		soundHandle = {};
		instance = nullptr;
		desc = nullptr;
		volume = 1.0f;
		playbackTime = 0;
	}

	void StopAndReset() noexcept
	{
		if (instance)
			instance->Stop();

		Reset();
	}
};
