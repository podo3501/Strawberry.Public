export module Service.Audio;

import std;
import :SoundRepository;
import :VoicePool;
import Contract.Audio.Assets;
import Contract.Audio.Service;
import Contract.Audio.Backend;
import Contract.IAssetAsyncLoader;

struct GroupInfo
{
	float volume{ 1.0f };
};

struct PendingSoundPlay
{
	VoiceHandle voice;
	SoundHandle sound;
};

class AudioService : public IAudioService
{
public:
	~AudioService() = default;
	AudioService(
		std::unique_ptr<IAudioBackend> audioBackend,
		const SoundAssetView& sndAssetView,
		IAssetAsyncLoader* asyncLoader) noexcept;
	bool Initialize(int maxVoices, int maxStreams) noexcept;

	virtual SoundHandle AcquireStaticSound(std::string_view soundID) override;
	virtual SoundHandle AcquireStreamSound(std::string_view soundID) override;

	virtual VoiceHandle Play(SoundHandle sh) noexcept override;
	virtual bool Pause(VoiceHandle vh) noexcept override;
	virtual bool Resume(VoiceHandle vh) noexcept override;
	virtual bool Stop(VoiceHandle vh) noexcept override;
	virtual bool AllStop() noexcept override;
	virtual void Update() noexcept override;
	virtual bool Unload(SoundHandle sh) noexcept override;

	virtual PlaybackState GetState(VoiceHandle vh) const noexcept override;
	virtual void SetMasterVolume(float volume) noexcept override;
	virtual float GetMasterVolume() const noexcept override;
	virtual bool SetVolume(VoiceHandle vh, float volume) noexcept override;

private:
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