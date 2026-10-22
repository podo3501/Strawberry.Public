export module Contract.Audio.Service:IAudioService;

import :VoiceHandle;
import :SoundHandle;
import Contract.Audio.Assets;
import Contract.Audio.Shared;
import Contract.Audio.Backend;
import Contract.IAssetAsyncLoader;

export struct IAudioService
{
	virtual SoundHandle AcquireStaticSound(std::string_view soundID) = 0;
	virtual SoundHandle AcquireStreamSound(std::string_view soundID) = 0;

	virtual VoiceHandle Play(SoundHandle sh) noexcept = 0;
	virtual bool Pause(VoiceHandle vh) noexcept = 0;
	virtual bool Resume(VoiceHandle vh) noexcept = 0;
	virtual bool Stop(VoiceHandle vh) noexcept = 0;
	virtual bool AllStop() noexcept = 0;
	virtual void Update() noexcept = 0;
	virtual bool Unload(SoundHandle sh) noexcept = 0;

	virtual PlaybackState GetState(VoiceHandle vh) const noexcept = 0;
	virtual void SetMasterVolume(float volume) noexcept = 0;
	virtual float GetMasterVolume() const noexcept = 0;
	virtual bool SetVolume(VoiceHandle vh, float volume) noexcept = 0;
};

export std::unique_ptr<IAudioService> CreateAudioService(
	std::unique_ptr<IAudioBackend> backend,
	const SoundAssetView& sndAssetView,
	IAssetAsyncLoader* asyncLoader,
	int maxVoices,
	int maxStreams) noexcept;