export module DxRender.Packet:UIMeshAppend;

import std;
import DxRender.Definition;
import Contract.Render.Backend.View;

// 이미지 아이템 하나를 CPU에서 world/uv를 구워 단일 batch buffer에 append한다.
// static mesh(quad 등)의 CPU 템플릿(StaticMeshResource::GetCPUTemplate)이 있는 아이템만 처리한다.
export void AppendUIItem(
    const DrawUIItem& ui,
    UIBatchBuffer& buffer);