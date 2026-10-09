export module Contract.Asset.Data:StreamSound;

import std;
import Core.TypeHierarchy;
import Contract.Asset.AssetData;
import Contract.Asset.Interfaces;

export struct StreamSoundAsset : public Core::TypeNode<StreamSoundAsset, AssetData>
{
	virtual ~StreamSoundAsset() = default;

	std::shared_ptr<IReadStream> stream;
};