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
	void Stop() override;
	void Update() override;
	bool SetVolume(float volume) override;
	PlaybackState GetState() const noexcept override;

private:
	bool PrepareStream();
	bool PushChunk();
	bool EnsureBound();

	AudioDevice* m_device{ nullptr };
	StreamSoundBuffer* m_buffer{ nullptr };
	std::unique_ptr<IReadStream> m_resourceStream;
	OggVorbis_File m_vorbisFile{};
	bool m_vorbisOpened{ false };

	SDL_AudioStream* m_stream{ nullptr };
	float m_volume{ 1.0f };
	bool m_loop{ false };
	bool m_draining{ false };
	int m_lowWatermarkBytes{ 0 }; // 스트림에 미리 쌓아 둘 양(약 0.5초). 소리 형식에 따라 바이트 수가 달라진다.
	PlaybackState m_state{ PlaybackState::Stopped };
	bool m_lent{ false }; // SetBuffer로 빌려준 뒤 Stop으로 돌려받기 전. 일시정지했거나 끝난 소리도 빌려준 상태다.

	std::array<char, 16384> m_decodeBuffer{};
};