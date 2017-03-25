#ifndef FASTREGEXP_UNICODE_H
#define FASTREGEXP_UNICODE_H

#include <locale>
#include <string>
#include <codecvt>

namespace FastRegExp
{
namespace Unicode
{
template <typename Facet>
struct DeletableFacet : Facet
{
    using Facet::Facet;
    ~DeletableFacet() {}
};

template <typename T>
static inline std::string toString(const T &v)
{
    return std::wstring_convert<
        DeletableFacet<
            std::codecvt<
                char32_t,
                char,
                std::mbstate_t
            >
        >,
        char32_t
    >().to_bytes(v);
}

template <typename T>
static inline std::u32string toUnicode(const T &v)
{
    return std::wstring_convert<
        DeletableFacet<
            std::codecvt<
                char32_t,
                char,
                std::mbstate_t
            >
        >,
        char32_t
    >().from_bytes(v);
}
}
}

#endif /* FASTREGEXP_UNICODE_H */
