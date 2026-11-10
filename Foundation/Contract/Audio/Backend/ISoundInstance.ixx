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
	// 정리이자 반납이라 실패를 돌려주지 않고 로그로 남긴다. 부른 뒤에는 백엔드가 이 인스턴스를 다시 빌려줄 수 있다.
	virtual void Stop() = 0;
	virtual void Update() = 0;
	virtual bool SetVolume(float volume) = 0;
	virtual PlaybackState GetState() const noexcept = 0;
};