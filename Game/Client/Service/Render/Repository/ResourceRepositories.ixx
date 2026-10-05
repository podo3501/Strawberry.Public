export module Client.Render.Repository:ResourceRepositories;

import std;
import :ResourceRepository;
import Client.Render.ResourceHandles;

export using BrushRepository = ResourceRepository<BrushTag>;
export using DebugMaterialRepository = ResourceRepository<DebugMaterialTag>;
export using DebugMeshRepository = ResourceRepository<DebugMeshTag>;
export using EnvironmentRepository = ResourceRepository<EnvironmentTag>;
export using FontRepository = ResourceRepository<FontTag>;
export using MeshRepository = ResourceRepository<MeshTag>;
export using MaterialRepository = ResourceRepository<MaterialTag>;