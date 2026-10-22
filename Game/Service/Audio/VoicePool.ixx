export module Service.Audio:VoicePool;

import std;
import :Voice;
import :LoadedSound;
import Core.Handle;
import Contract.Audio.Assets;
import Contract.Audio.Service;
import Contract.Audio.Backend;

export class VoicePool
{
public:
	~VoicePool() = default;
	VoicePool() = delete;
	explicit VoicePool(IAudioBackend* audioBackend);

	bool Setup(int maxVoices, int maxStreams) noexcept;
	VoiceHandle AcquireVoiceHandle(const SoundDesc* desc) noexcept;

	VoiceHandle Play(SoundHandle sh, const LoadedSound* loaded, const PlaybackParams& params) noexcept;
	bool Play(VoiceHandle vh, SoundHandle sh, const LoadedSound* loaded, const PlaybackParams& params) noexcept;
	bool Pause(VoiceHandle vh) noexcept;
	bool Resume(VoiceHandle vh) noexcept;
	bool StopVoice(VoiceHandle vh) noexcept;
	bool StopVoices(SoundHandle sh) noexcept;
	bool StopVoices(SoundType type) noexcept;
	bool SetVolume(VoiceHandle vh, float volume) noexcept;
	void UpdateVoices() noexcept;

	PlaybackState GetState(VoiceHandle vh) const noexcept;
	const SoundDesc* GetDesc(VoiceHandle vh) const noexcept;

private:
	ISoundInstance* CreateInstance(const LoadedSound* loaded);
	VoiceHandle StealAndAcquire(const SoundDesc* desc, std::vector<Voice*>& stealList) noexcept;
	void ActivateVoice(VoiceHandle vh, SoundHandle sh, ISoundInstance* instance, const SoundDesc* desc) noexcept;

	const ISoundInstance* GetInstance(VoiceHandle vh) const noexcept;
	ISoundInstance* GetInstance(VoiceHandle vh) noexcept;

private:
	IAudioBackend* m_audioBackend{ nullptr };
	int m_maxVoices{ 0 };
	int m_maxStreams{ 0 };
	static constexpr int MaxHandles{ 64 };

	Core::FixedHandlePool<Voice, VoiceTag, MaxHandles> m_voices;
	std::vector<Voice*> m_stealStatics;
	std::vector<Voice*> m_stealStreams;
};