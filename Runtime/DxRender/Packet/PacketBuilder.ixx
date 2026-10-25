export module DxRender.Packet:PacketBuilder;

import DxRender.Definition;
import DxRender.Text;
import DxRender.TransientMesh;
import Core.Math;
import Contract.Render.Backend.View;

export FramePacket BuildPacket(
    SceneFrameData& frame,
    TextSystem& textSystem,
    TransientMesh& transientMesh,
    const Core::Size& screenSize);