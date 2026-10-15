module SDLAudio;

import :StreamSoundBuffer;
import Core.TypeHierarchy;
import Contract.Asset.Data;

bool StreamSoundBuffer::LoadFromAsset(std::shared_ptr<AssetData> asset)
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