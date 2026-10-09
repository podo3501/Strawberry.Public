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

    void Clear()
    {
        m_cache.clear(); // shared_ptr들의 참조 카운트만 줄어듦. 아직 살아있는 weak_ptr은 자동으로 expired 처리됨
    }

private:
    struct CacheKey
    {
        FontResource* fontRes{ nullptr }; // 검색 속도를 극대화하기 위해 폰트 포인터(Raw주소)와 코드포인트를 64비트 키 하나로 결합
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