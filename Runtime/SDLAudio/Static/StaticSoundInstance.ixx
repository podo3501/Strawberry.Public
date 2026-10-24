module;

#include "SDL3/SDL_properties.h"
#include "SDL3_mixer/SDL_mixer.h"

export module SDLAudio:StaticSoundInstance;

import :StaticSoundBuffer;
import Contract.Audio.Backend;

export class StaticSoundInstance : public ISoundInstance
{
public:
	~StaticSoundInstance() override;
	StaticSoundInstance() = default;
	
	bool Setup(MIX_Mixer* mixer);
	bool SetBuffer(StaticSoundBuffer* buffer);
	
	bool Reset(const PlaybackParams& params) override;
	bool Play() override;
	bool Pause() override;
	bool Resume() override;
	bool Stop() override;
	void Update() override;
	bool SetVolume(float volume) override;
	PlaybackState GetState() const noexcept override;

	void OnStopped();

private:
	MIX_Track* m_track{ nullptr };
	SDL_PropertiesID m_options{};
	PlaybackState m_state{ PlaybackState::Stopped };
};