export module Service.Audio:AudioService;

import std;
import :VoiceHandle;
import :SoundHandle;
import :SoundRepository;
import :VoicePool;
import Contract.Asset.Data;
import Contract.Audio.Interfaces;
import Service.IAssetAsyncLoader;

struct GroupInfo
{
	float volume{ 1.0f };
};

struct PendingSoundPlay
{
	VoiceHandle voice;
	SoundHandle sound;
};

export class AudioService
{
public:
	~AudioService() = default;
	AudioService() = delete;

	static std::unique_ptr<AudioService> Create(
		const SoundAssetView& sndAssetView,
		std::unique_ptr<IAudioBackend> backend,
		IAssetAsyncLoader* asyncLoader,
		int maxVoices,
		int maxStreams) noexcept;

	SoundHandle AcquireStaticSound(std::string_view soundID);
	SoundHandle AcquireStreamSound(std::string_view soundID);

	VoiceHandle Play(SoundHandle sh) noexcept;
	bool Pause(VoiceHandle vh) noexcept;
	bool Resume(VoiceHandle vh) noexcept;
	bool Stop(VoiceHandle vh) noexcept;
	bool AllStop() noexcept;
	void Update() noexcept;
	bool Unload(SoundHandle sh) noexcept;

	PlaybackState GetState(VoiceHandle vh) const noexcept;
	void SetMasterVolume(float volume) noexcept;
	float GetMasterVolume() const noexcept;
	bool SetVolume(VoiceHandle vh, float volume) noexcept;

private:
	AudioService(
		std::unique_ptr<IAudioBackend> audioBackend,
		const SoundAssetView& sndAssetView,
		IAssetAsyncLoader* asyncLoader) noexcept;

	bool Initialize(int maxVoices, int maxStreams) noexcept;
	void CreateAudioGroup() noexcept;

	PlaybackParams GetParams(const SoundDesc* desc) noexcept;
	VoiceHandle EnqueueDeferred(SoundHandle sh, const SoundDesc* desc);
	void FlushPending();

	float GetGroupVolume(AudioGroup group) const noexcept;
	float GetInstanceVolume(AudioGroup group, float volume) const noexcept;

private:
	std::unique_ptr<IAudioBackend> m_audioBackend;
	SoundAssetView m_sndAssetView;
	SoundRepository m_repository;
	VoicePool m_voicePool;

	float m_masterVolume{ 1.0f };
	std::unordered_map<AudioGroup, GroupInfo> m_groupInfos;
	std::vector<PendingSoundPlay> m_pendingPlay;
};