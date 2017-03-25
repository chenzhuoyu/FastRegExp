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

struct Item;
struct Range;
struct SubExpr;
struct Character;

/** Node implementations **/

struct RegExp final : public Node
{
    std::vector<std::shared_ptr<Section>> sections;
};

struct Section final : public Node
{
    std::vector<std::shared_ptr<Elementry>> elements;
};

struct Elementry final : public Node
{
    enum class Type : int
    {
        Any,
        Set,
        Group,
        Character,
        EndOfString,
    };

public:
    struct Modifier
    {
        enum class Type : int
        {
            None,
            Plus,
            Star,
            Repeat,
            Question,
        };

    public:
        Type type = Type::None;
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
    std::shared_ptr<Character> character;

};

struct Range final : public Node
{
    bool isInverted = false;
    std::vector<std::pair<wchar_t, wchar_t>> items;
};

struct SubExpr final : public Node
{
    enum class Type : int
    {
        Simple,
        NonCapture,
        PositiveAssert,
        NegativeAssert,
        ReversePositiveAssert,
        ReverseNegativeAssert,
    };

public:
    Type type;
    std::shared_ptr<RegExp> expr;

};

struct Character final : public Node
{
    enum class Type : int
    {
        Group,
        Simple,

        Word,
        Digit,
        Space,
        Border,
        Control,

        NonWord,
        NonDigit,
        NonSpace,
        NonBorder,
    };

public:
    Type type;
    size_t group;
    wchar_t character;

};
}
}

#endif /* FASTREGEXP_AST_H */
