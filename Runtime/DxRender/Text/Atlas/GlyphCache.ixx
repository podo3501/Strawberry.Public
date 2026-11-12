export module DxRender.Text:GlyphCache;

import std;
import :TextTypes;
import DxRender.Resource;
import Core.Utils;

export class GlyphCache
{
public:
    void Insert(FontResource* fontRes, std::uint32_t glyphIndex, std::uint32_t size, const GlyphInfo& info)
    {
        CacheKey key{ fontRes, glyphIndex, size };
        m_cache[key] = info;
    }

    const GlyphInfo* Get(FontResource* fontRes, std::uint32_t glyphIndex, std::uint32_t size) const
    {
        CacheKey key{ fontRes, glyphIndex, size };
        auto it = m_cache.find(key);
        if (it != m_cache.end())
            return &it->second;

        return nullptr;
    }

private:
    struct CacheKey
    {
        FontResource* fontRes{ nullptr }; // 폰트 포인터(Raw 주소), 글리프 번호, 글자 크기 세 값이 키이다.
        std::uint32_t glyphIndex{ 0 };
        std::uint32_t size{ 0 };

        bool operator==(const CacheKey&) const = default;
    };

    struct CacheKeyHash
    {
        std::size_t operator()(const CacheKey& key) const
        {
            return Core::HashOf(
                key.fontRes,
                key.glyphIndex,
                key.size);
        }
    };

    std::unordered_map<CacheKey, GlyphInfo, CacheKeyHash> m_cache;
};