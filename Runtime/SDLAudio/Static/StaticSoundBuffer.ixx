module;

#include "SDL3_mixer/SDL_mixer.h"

export module SDLAudio:StaticSoundBuffer;

import std;
import Contract.Audio.Backend;
import Contract.Common.Asset;

export class StaticSoundBuffer : public ISoundBuffer
{
public:
	~StaticSoundBuffer() override;
	StaticSoundBuffer() = delete;
	explicit StaticSoundBuffer(MIX_Mixer* mixer) noexcept;

	bool LoadFromAsset(std::shared_ptr<Asset> asset) override;
	MIX_Audio* GetAudio() const noexcept;

private:
	MIX_Mixer* m_mixer{ nullptr };
	MIX_Audio* m_audio{ nullptr };
};