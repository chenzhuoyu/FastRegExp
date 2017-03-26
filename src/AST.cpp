#include <iomanip>
#include <sstream>
#include <algorithm>

#include "AST.h"
#include "Unicode.h"

template <typename T>
static inline std::string hex(T &&value)
{
    std::stringstream stream;
    stream << "0x"
           << std::setfill('0')
           << std::setw(sizeof(T) * 2)
           << std::hex
           << static_cast<uintptr_t>(value);
    return stream.str();
}

static inline std::string header(size_t count)
{
    std::string result;
    result.reserve(count * 2);
    while (count--) result += "| ";
    return result;
}

std::string FastRegExp::AST::RegExp::toString(size_t level) const noexcept
{
    std::string result = header(level) + "RegExp\n";
    std::for_each(sections.begin(), sections.end(), [&](auto x){ result += x->toString(level + 1); });
    return result;
}

std::string FastRegExp::AST::Section::toString(size_t level) const noexcept
{
    std::string result = header(level) + "Section\n";
    std::for_each(elements.begin(), elements.end(), [&](auto x){ result += x->toString(level + 1); });
    return result;
}

std::string FastRegExp::AST::Elementry::toString(size_t level) const noexcept
{
    std::string result = header(level) + "Elementry\n";

    switch (type)
    {
        case Type::ElementryAny           : result += header(level + 1) + "Any\n";           break;
        case Type::ElementryEndOfString   : result += header(level + 1) + "EndOfString\n";   break;
        case Type::ElementryStartOfString : result += header(level + 1) + "StartOfString\n"; break;

        case Type::ElementryRange         : result += range->toString(level + 1);     break;
        case Type::ElementrySubExpr       : result += subexpr->toString(level + 1);   break;
        case Type::ElementryCharacter     : result += character->toString(level + 1); break;
    }

    switch (modifier.type)
    {
        case Modifier::Type::ModifierNone     : result += header(level + 1) + "Modifier None\n"; break;
        case Modifier::Type::ModifierPlus     : result += header(level + 1) + (modifier.isLazy ? "Modifier '+' Lazy\n" : "Modifier '+'\n"); break;
        case Modifier::Type::ModifierStar     : result += header(level + 1) + (modifier.isLazy ? "Modifier '*' Lazy\n" : "Modifier '*'\n"); break;
        case Modifier::Type::ModifierQuestion : result += header(level + 1) + (modifier.isLazy ? "Modifier '?' Lazy\n" : "Modifier '?'\n"); break;

        case Modifier::Type::ModifierRepeat:
        {
            result += header(level + 1);
            result += modifier.isLazy ? "Modifier Repeat Lazy\n" : "Modifier Repeat\n";
            result += header(level + 2);
            result += "Lower " + std::to_string(modifier.lower) + "\n";

            if (modifier.upper < 0)
            {
                result += header(level + 2);
                result += "Upper Infinite\n";
            }
            else
            {
                result += header(level + 2);
                result += "Upper " + std::to_string(modifier.upper) + "\n";
            }

            break;
        }
    }

    return result;
}

std::string FastRegExp::AST::Range::toString(size_t level) const noexcept
{
    std::string result = header(level)
        + (isInverted ? "Range Inverted " : "Range ")
        + std::to_string(items.size())
        + "\n";

    for (const auto &range : items)
    {
        if (range.second == nullptr)
        {
            result += header(level + 1);
            result += "Item Single\n";
            result += range.first->toString(level + 2);
        }
        else
        {
            result += header(level + 1);
            result += "Item Range\n";
            result += header(level + 2);
            result += "Lower\n";
            result += range.first->toString(level + 3);
            result += header(level + 2);
            result += "Upper\n";
            result += range.second->toString(level + 3);
        }
    }

    return result;
}

std::string FastRegExp::AST::SubExpr::toString(size_t level) const noexcept
{
    std::string result = header(level) + "Sub-expression\n";

    switch (type)
    {
        case Type::SubExprSimple             : result += header(level + 1) + "Simple\n"            ; break;
        case Type::SubExprNonCapture         : result += header(level + 1) + "NonCapture\n"        ; break;
        case Type::SubExprPositiveLookahead  : result += header(level + 1) + "PositiveLookahead\n" ; break;
        case Type::SubExprNegativeLookahead  : result += header(level + 1) + "NegativeLookahead\n" ; break;
        case Type::SubExprPositiveLookbehind : result += header(level + 1) + "PositiveLookbehind\n"; break;
        case Type::SubExprNegativeLookbehind : result += header(level + 1) + "NegativeLookbehind\n"; break;
    }

    result += expr->toString(level + 2);
    return result;
}

std::string FastRegExp::AST::Character::toString(size_t level) const noexcept
{
    std::string result = header(level);

    switch (type)
    {
        case Type::CharacterWord      : result += "Word\n"  ; break;
        case Type::CharacterDigit     : result += "Digit\n" ; break;
        case Type::CharacterSpace     : result += "Space\n" ; break;
        case Type::CharacterBorder    : result += "Border\n"; break;

        case Type::CharacterNonWord   : result += "Non-Word\n"  ; break;
        case Type::CharacterNonDigit  : result += "Non-Digit\n" ; break;
        case Type::CharacterNonSpace  : result += "Non-Space\n" ; break;
        case Type::CharacterNonBorder : result += "Non-Border\n"; break;

        case Type::CharacterGroup:
        {
            result += "Group ";
            result += std::to_string(group);
            result += "\n";
            break;
        }

        case Type::CharacterSimple:
        {
            result += "Simple '";
            result += Unicode::toString(character);
            result += "'(Hex: ";
            result += hex(character);
            result += ")\n";
            break;
        }

        case Type::CharacterControl:
        {
            result += "Control '";
            result += Unicode::toString(character);
            result += "'(Hex: ";
            result += hex(character);
            result += ")\n";
            break;
        }
    }

    return result;
}
