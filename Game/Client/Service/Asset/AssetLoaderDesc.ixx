export module Client.Asset.Service:AssetLoaderDesc;

import std;
import Client.Asset.Contract;
import Core.TypeHierarchy;

namespace Client
{
	export struct AssetLoaderDesc
	{
		Core::TypeID type;
		std::string extension;
		std::unique_ptr<IAssetLoader> loader;

		template<typename T>
		static AssetLoaderDesc Make(std::string_view ext, std::unique_ptr<IAssetLoader> loader)
		{
			return { Core::GetTypeID<T>(), std::string(ext), std::move(loader) };
		}
	};
}