export module Client.Asset.Contract:AssetLoaderDesc;

import std;
import :IAssetLoader;
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