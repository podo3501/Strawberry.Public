export module DxRender.Packet:UIItemBuilder;

import std;
import DxRender.Definition;
import DxRender.Text;
import DxRender.TransientMesh;
import Contract.Render.Backend.View;

export std::optional<RenderUIItem> BuildUIItems(
    OverlayViewDrawList& draws,
    TextSystem& textSystem,
    TransientMesh& transientMesh);