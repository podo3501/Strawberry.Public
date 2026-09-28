module;

#include "SDL3_mixer/SDL_mixer.h"

export module Runtime.Audio:StaticSoundBuffer;

import std;
import Core.TypeHierarchy;
import Client.Audio.Interfaces;
import Client.Asset.Data;

export class StaticSoundBuffer : public ISoundBuffer
{
public:
	StaticSoundBuffer() = delete;
	explicit StaticSoundBuffer(MIX_Mixer* mixer) noexcept :
		m_mixer{ mixer },
		m_audio{ nullptr }
	{}

	~StaticSoundBuffer() override
	{
		if (m_audio)
		{
			MIX_DestroyAudio(m_audio);
		}
	}

	bool LoadFromAsset(std::shared_ptr<AssetData> asset) override
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

	MIX_Audio* GetAudio() const noexcept { return m_audio; }

private:
	MIX_Mixer* m_mixer{ nullptr };
	MIX_Audio* m_audio{ nullptr };
};