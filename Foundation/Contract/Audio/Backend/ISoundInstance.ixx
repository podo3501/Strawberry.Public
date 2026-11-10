export module Contract.Audio.Backend:ISoundInstance;

import std;
import Contract.Audio.Shared;
import :PlaybackParams;

export struct ISoundInstance
{
	virtual ~ISoundInstance() = default;

	virtual bool Reset(const PlaybackParams& params) = 0;
	virtual bool Play() = 0;
	virtual bool Pause() = 0;
	virtual bool Resume() = 0;
	virtual void Stop() = 0; // 정리 요청이라 실패를 돌려주지 않는다. 실패는 로그로 남기고 끝난 뒤에는 다시 쓸 수 있는 상태가 된다.
	virtual void Update() = 0;
	virtual bool SetVolume(float volume) = 0;
	virtual PlaybackState GetState() const noexcept = 0;
};