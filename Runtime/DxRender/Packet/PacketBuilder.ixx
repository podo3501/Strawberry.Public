export module DxRender.Packet:PacketBuilder;

import DxRender.Definition;
import DxRender.Text;
import DxRender.Provider;
import Core.Math;
import Contract.Render.View;

export FramePacket BuildPacket(
    SceneFrameData& frame,
    TextSystem& textSystem,
    TransientMeshProvider& meshProvider,
    const Core::Size& screenSize);