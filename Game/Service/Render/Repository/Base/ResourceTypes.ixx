export module Service.Render.Repository:ResourceTypes;

import std;
import Core.Utils;

export enum class LoadState
{
    Pending,
    AssetLoading,
    ResourceLoading,
    Ready,
    Failed
};