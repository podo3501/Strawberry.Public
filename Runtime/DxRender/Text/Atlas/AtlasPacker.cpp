module;

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

module DxRender.Text;

import :AtlasPacker;
import Core.Assert;

void AtlasPacker::Initialize(const Core::Size& textureSize)
{
    Reset();

    m_textureSize = textureSize;
    m_nodes.resize(textureSize.width); // Skyline 알고리즘은 너비(width)만큼의 노드 공간이 필요합니다.

    stbrp_init_target(
        &m_context,
        textureSize.width,
        textureSize.height,
        m_nodes.data(),
        static_cast<int>(m_nodes.size())); // stb 패커 초기화

    m_initialized = true;
}

std::optional<Core::Point> AtlasPacker::AllocateRect(const Core::Size& size)
{
    Core::Assert(m_initialized);

    stbrp_rect rect{};
    rect.w = size.width;
    rect.h = size.height;

    stbrp_pack_rects(&m_context, &rect, 1); // 사각형 1개 패킹 시도
    if (!rect.was_packed) // 공간 부족으로 패킹 실패 (아틀라스가 가득 참)
        return std::nullopt;

    return Core::Point{ rect.x, rect.y };
}

void AtlasPacker::Reset()
{
    m_context = {};
    m_nodes.clear();
    m_initialized = false;
}