export module Client.Asset.Data:StreamSound;

import std;
import Core.TypeHierarchy;
import Client.Asset.AssetData;
import Client.Asset.Interfaces;

export struct StreamSoundAsset : public Core::TypeNode<StreamSoundAsset, AssetData>
{
	virtual ~StreamSoundAsset() = default;

	std::shared_ptr<IReadStream> stream;
};