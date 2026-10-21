module SDLAudio;

import :StreamSoundBuffer;
import Core.TypeHierarchy;
import Contract.Audio.Assets;

bool StreamSoundBuffer::LoadFromAsset(std::shared_ptr<Asset> asset)
{
	auto streamAsset = Core::Cast<StreamSoundAsset>(asset);
	if (!streamAsset)
		return false;

	m_fileStream = streamAsset->stream;
	return true;
}

IReadStream* StreamSoundBuffer::GetStream() noexcept
{
	return m_fileStream.get();
}