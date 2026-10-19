module;

#include "stb_rect_pack.h"

export module DxRender.Text:AtlasPacker;

import std;
import Core.Math;

export class AtlasPacker
{
public:
    ~AtlasPacker() = default;
    AtlasPacker() = default;

    AtlasPacker(const AtlasPacker&) = delete;
    AtlasPacker& operator=(const AtlasPacker&) = delete;

    AtlasPacker(AtlasPacker&&) = delete;
    AtlasPacker& operator=(AtlasPacker&&) = delete;

    void Initialize(const Core::Size& textureSize);
    std::optional<Core::Point> AllocateRect(const Core::Size& size);
    void Reset();

private:
    Core::Size m_textureSize{};
    stbrp_context m_context;
    std::vector<stbrp_node> m_nodes;

    bool m_initialized{ false };
};