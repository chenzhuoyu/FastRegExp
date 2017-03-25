#ifndef FASTREGEXP_AST_H
#define FASTREGEXP_AST_H

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <sys/types.h>

namespace FastRegExp
{
namespace AST
{
class Node
{
    ssize_t _pos = -1;

public:
    virtual ~Node() {}
    explicit Node() {}

public:
    virtual std::string toString(size_t level = 0) const noexcept = 0;

private:
    template <typename NodeType>
    inline NodeType *setPosition(ssize_t pos)
    {
        _pos = pos;
        return static_cast<NodeType *>(this);
    }

public:
    template <typename NodeType, typename ... Args>
    static std::shared_ptr<NodeType> create(ssize_t pos, Args && ... args)
    {
        static_assert(std::is_convertible<NodeType *, Node *>::value, "`NodeType *` must be convertiable to `Node *`");
        return std::shared_ptr<NodeType>((new NodeType(std::forward<Args>(args) ...))->template setPosition<NodeType>(pos));
    }
};

/** Forward declarations **/

struct RegExp;
struct Section;
struct Elementry;

struct Range;
struct SubExpr;
struct Character;

/** Node implementations **/

struct RegExp final : public Node
{
    std::vector<std::shared_ptr<Section>> sections;

public:
    virtual std::string toString(size_t level) const noexcept;

};

struct Section final : public Node
{
    std::vector<std::shared_ptr<Elementry>> elements;

public:
    virtual std::string toString(size_t level) const noexcept;

};

struct Elementry final : public Node
{
    enum class Type : int
    {
        ElementryAny,
        ElementryRange,
        ElementrySubExpr,
        ElementryCharacter,
        ElementryEndOfString,
        ElementryStartOfString,
    };

public:
    struct Modifier
    {
        enum class Type : int
        {
            ModifierNone,
            ModifierPlus,
            ModifierStar,
            ModifierRepeat,
            ModifierQuestion,
        };

    public:
        Type type = Type::ModifierNone;
        bool isLazy = false;

    public:
        size_t lower = 0;
        ssize_t upper = -1;

    };

public:
    Type type;
    Modifier modifier;

public:
    std::shared_ptr<Range> range;
    std::shared_ptr<SubExpr> subexpr;
    std::shared_ptr<Character> character;

public:
    virtual std::string toString(size_t level) const noexcept;

};

struct Range final : public Node
{
    bool isInverted = false;
    std::vector<std::pair<std::shared_ptr<Character>, std::shared_ptr<Character>>> items;

public:
    virtual std::string toString(size_t level) const noexcept;

};

struct SubExpr final : public Node
{
    enum class Type : int
    {
        SubExprSimple,
        SubExprNonCapture,
        SubExprPositiveLookahead,
        SubExprNegativeLookahead,
        SubExprPositiveLookbehind,
        SubExprNegativeLookbehind,
    };

public:
    Type type;
    std::shared_ptr<RegExp> expr;

public:
    virtual std::string toString(size_t level) const noexcept;

};

struct Character final : public Node
{
    enum class Type : int
    {
        CharacterGroup,
        CharacterSimple,
        CharacterControl,

        CharacterWord,
        CharacterDigit,
        CharacterSpace,
        CharacterBorder,

        CharacterNonWord,
        CharacterNonDigit,
        CharacterNonSpace,
        CharacterNonBorder,
    };

public:
    Type type;

public:
    union
    {
        size_t group;
        char32_t character;
    };

public:
    virtual std::string toString(size_t level) const noexcept;

};
}
}

#endif /* FASTREGEXP_AST_H */
