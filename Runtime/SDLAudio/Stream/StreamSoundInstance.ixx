module;

#include "SDL3/SDL.h"
#include "vorbis/vorbisfile.h"

export module SDLAudio:StreamSoundInstance;

import std;
import :Device;
import :StreamSoundBuffer;
import Contract.Audio.Backend;
import Contract.Asset.Backend;

export class StreamSoundInstance : public ISoundInstance
{
public:
	~StreamSoundInstance() override;
	StreamSoundInstance() = default;

	bool Setup(AudioDevice* device);
	bool SetBuffer(StreamSoundBuffer* buffer);

	bool Reset(const PlaybackParams& params) override;
	bool Play() override;
	bool Pause() override;
	bool Resume() override;
	bool Stop() override;
	void Update() override;
	bool SetVolume(float volume) override;
	PlaybackState GetState() const noexcept override;

private:
	bool PrepareStream();
	bool PushChunk();

	AudioDevice* m_device{ nullptr };
	StreamSoundBuffer* m_buffer{ nullptr };
	std::unique_ptr<IReadStream> m_resourceStream;
	OggVorbis_File m_vorbisFile{};
	bool m_vorbisOpened{ false };

	SDL_AudioStream* m_stream{ nullptr };
	float m_volume{ 1.0f };
	bool m_loop{ false };
	bool m_draining{ false };
	PlaybackState m_state{ PlaybackState::Stopped };

	std::array<char, 16384> m_decodeBuffer{};
};