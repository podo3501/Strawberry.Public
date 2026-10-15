module;

#include "SDL3/SDL_properties.h"
#include "SDL3_mixer/SDL_mixer.h"

module SDLAudio;

import :StaticSoundInstance;
import std;
import Core.Assert;
import Core.Utils;

StaticSoundInstance::~StaticSoundInstance()
{
	if (m_track)
		MIX_DestroyTrack(m_track);

	if (m_options)
		SDL_DestroyProperties(m_options);
}

bool StaticSoundInstance::Setup(MIX_Mixer* mixer)
{
	m_track = MIX_CreateTrack(mixer);
	if (!m_track)
		return false;

	m_options = SDL_CreateProperties();
	if (!m_options)
	{
		MIX_DestroyTrack(m_track);
		m_track = nullptr;
		return false;
	}

	MIX_SetTrackStoppedCallback(m_track, [](void* userdata, MIX_Track* track) {
		if (auto* self = static_cast<StaticSoundInstance*>(userdata))
			self->OnStopped();
		}, this);

	return true;
}

bool StaticSoundInstance::SetBuffer(StaticSoundBuffer* buffer)
{
	if (m_state == PlaybackState::Playing)
		return false;
	if (!m_track || !buffer)
		return false;

	return MIX_SetTrackAudio(m_track, buffer->GetAudio());
}

bool StaticSoundInstance::Reset(const PlaybackParams& params)
{
	if (!MIX_StopTrack(m_track, 0)) return false;
	if (!MIX_SetTrackPlaybackPosition(m_track, 0)) return false;
	if (!SetVolume(params.volume)) return false;

	m_state = PlaybackState::Stopped;

	return true;
}

bool StaticSoundInstance::Play()
{
	if (!MIX_PlayTrack(m_track, m_options)) return false;
	m_state = PlaybackState::Playing;

	return true;
}

bool StaticSoundInstance::Pause()
{
	if (m_state != PlaybackState::Playing)
		return false;

	if (!MIX_PauseTrack(m_track)) return false;
	m_state = PlaybackState::Paused;

	return true;
}

bool StaticSoundInstance::Resume()
{
	if (m_state != PlaybackState::Paused)
		return false;

	if (!MIX_ResumeTrack(m_track)) return false;
	m_state = PlaybackState::Playing;

	return true;
}

bool StaticSoundInstance::Stop()
{
	return MIX_StopTrack(m_track, 0);
}

void StaticSoundInstance::Update()
{}

bool StaticSoundInstance::SetVolume(float volume)
{
	volume = std::clamp(volume, 0.0f, 1.0f);
	return MIX_SetTrackGain(m_track, volume);
}

PlaybackState StaticSoundInstance::GetState() const noexcept
{
	if (m_track == nullptr)
		return Core::InvalidEnum<PlaybackState>;
	return m_state;
}

void StaticSoundInstance::OnStopped()
{
	auto isOk = MIX_SetTrackPlaybackPosition(m_track, 0);
	Core::Assert(isOk);
	m_state = PlaybackState::Stopped;
}