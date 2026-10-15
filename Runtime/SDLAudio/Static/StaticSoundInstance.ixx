module;

#include "SDL3/SDL_properties.h"
#include "SDL3_mixer/SDL_mixer.h"

export module SDLAudio:StaticSoundInstance;

import :StaticSoundBuffer;
import Contract.Audio.Interfaces;

export class StaticSoundInstance : public ISoundInstance
{
public:
	StaticSoundInstance() = default;
	virtual ~StaticSoundInstance() override;
	bool Setup(MIX_Mixer* mixer);
	bool SetBuffer(StaticSoundBuffer* buffer);
	
	virtual bool Reset(const PlaybackParams& params) override;
	virtual bool Play() override;
	virtual bool Pause() override;
	virtual bool Resume() override;
	virtual bool Stop() override;
	virtual void Update() override;
	virtual bool SetVolume(float volume) override;
	virtual PlaybackState GetState() const noexcept override;

	void OnStopped();

private:
	MIX_Track* m_track{ nullptr };
	SDL_PropertiesID m_options{};
	PlaybackState m_state{ PlaybackState::Stopped };
};