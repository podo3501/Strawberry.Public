export module Contract.Audio.Assets:StreamSound;

import std;
import Core.TypeHierarchy;
import Contract.Common.Asset;
import Contract.Asset.Shared;

export struct StreamSoundAsset : public Core::TypeNode<StreamSoundAsset, Asset>
{
	virtual ~StreamSoundAsset() = default;

	std::shared_ptr<IReadStream> stream;
};