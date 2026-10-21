module;

#include "SDL3_mixer/SDL_mixer.h"

module SDLAudio;

import :StaticSoundBuffer;
import Core.TypeHierarchy;
import Contract.Audio.Assets;

StaticSoundBuffer::StaticSoundBuffer(MIX_Mixer * mixer) noexcept 
	: m_mixer{ mixer }
	, m_audio{ nullptr }
{}

StaticSoundBuffer::~StaticSoundBuffer()
{
	if (m_audio)
	{
		MIX_DestroyAudio(m_audio);
	}
}

bool StaticSoundBuffer::LoadFromAsset(std::shared_ptr<Asset> asset)
{
	auto staticAsset = Core::Cast<StaticSoundAsset>(asset);
	if (!staticAsset)
		return false;

	if (m_audio)
	{
		MIX_DestroyAudio(m_audio);
		m_audio = nullptr;
	}

	SDL_AudioSpec spec{};
	spec.freq = staticAsset->sampleRate;
	spec.channels = staticAsset->channels;

	switch (staticAsset->format)
	{
	case SampleFormat::Int16:
		spec.format = SDL_AUDIO_S16;
		break;
	case SampleFormat::Float32:
		spec.format = SDL_AUDIO_F32;
		break;
	default:
		return false;
	}

	m_audio = MIX_LoadRawAudio(m_mixer, staticAsset->data.data(), staticAsset->data.size(), &spec);

	return m_audio != nullptr;
}

MIX_Audio* StaticSoundBuffer::GetAudio() const noexcept { return m_audio; }