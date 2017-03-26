#include <locale>
#include "Parser.h"

namespace FastRegExp
{
size_t Parser::parseInt(void)
{
    size_t x;
    char32_t ch;

    switch ((ch = next()))
    {
        case U'0':
            return 0;

        case U'1' ... U'9':
        {
            x = ch - U'0';
            break;
        }

        default:
            throw SyntaxError(_pos, "Invalid digit '" + Unicode::toString(ch) + "'");
    }

    while (isDigit(peek()))
    {
        x *= 10;
        x += next() - U'0';
    }

    return x;
}

std::shared_ptr<AST::RegExp> Parser::parseRegExp(char32_t delim)
{
    char32_t ch;
    std::shared_ptr<AST::RegExp> result = AST::Node::create<AST::RegExp>(_pos);

    for (;;)
    {
        /* parse one section */
        result->sections.push_back(parseSection(delim));

        /* splitted by '|' or custom delimeter */
        switch ((ch = peek()))
        {
            case 0:
                return result;

            case U'|':
            {
                next();
                break;
            }

            default:
            {
                if (ch != delim)
                    break;
                else
                    return result;
            }
        }
    }
}

std::shared_ptr<AST::Section> Parser::parseSection(char32_t delim)
{
    char32_t ch;
    std::shared_ptr<AST::Section> result = AST::Node::create<AST::Section>(_pos);

    for (;;)
    {
        /* splitted by '|' or custom delimeter */
        switch ((ch = peek()))
        {
            case 0:
            case U'|':
                return result;

            default:
            {
                if (ch == delim)
                    return result;

                /* parse one element */
                result->elements.push_back(parseElementry());
                break;
            }
        }
    }
}

std::shared_ptr<AST::Elementry> Parser::parseElementry(void)
{
    std::shared_ptr<AST::Elementry> result = AST::Node::create<AST::Elementry>(_pos);

    switch (peek())
    {
        case U'.': { next(); result->type = AST::Elementry::Type::ElementryAny          ; break; }
        case U'$': { next(); result->type = AST::Elementry::Type::ElementryEndOfString  ; break; }
        case U'^': { next(); result->type = AST::Elementry::Type::ElementryStartOfString; break; }

        case U'[':
        {
            result->type = AST::Elementry::Type::ElementryRange;
            result->range = parseRange();
            break;
        }

        case U'(':
        {
            result->type = AST::Elementry::Type::ElementrySubExpr;
            result->subexpr = parseSubExpr();
            break;
        }

        default:
        {
            result->type = AST::Elementry::Type::ElementryCharacter;
            result->character = parseCharacter();
            break;
        }
    }

    switch (peek())
    {
        case U'+': { next(); result->modifier.type = AST::Elementry::Modifier::Type::ModifierPlus    ; break; }
        case U'*': { next(); result->modifier.type = AST::Elementry::Modifier::Type::ModifierStar    ; break; }
        case U'?': { next(); result->modifier.type = AST::Elementry::Modifier::Type::ModifierQuestion; break; }

        case U'{':
        {
            next();
            result->modifier.type = AST::Elementry::Modifier::Type::ModifierRepeat;
            result->modifier.lower = parseInt();

            if (!skipIf<U','>())
                result->modifier.upper = result->modifier.lower;
            else if (peek() != U'}')
                result->modifier.upper = parseInt();

            if (next() != U'}')
                throw SyntaxError(_pos, "'}' expected");

            break;
        }

        default:
            break;
    }

    if (result->modifier.type != AST::Elementry::Modifier::Type::ModifierNone)
        result->modifier.isLazy = skipIf<U'?'>();

    return result;
}

std::shared_ptr<AST::Range> Parser::parseRange(void)
{
    std::shared_ptr<AST::Range> result = AST::Node::create<AST::Range>(_pos);

    if (next() != U'[')
        throw SyntaxError(_pos, "'[' expected");

    if (skipIf<U'^'>())
        result->isInverted = true;

    do
    {
        std::shared_ptr<AST::Character> upper = nullptr;
        std::shared_ptr<AST::Character> lower = parseCharacter();

        if (!skipIf<U'-'>())
        {
            /* single character */
            result->items.push_back(std::make_pair(std::move(lower), std::move(upper)));
        }
        else if (peek() != U']')
        {
            /* lower bounds and upper bounds */
            upper = parseCharacter();
            result->items.push_back(std::make_pair(std::move(lower), std::move(upper)));
        }
        else
        {
            upper = AST::Node::create<AST::Character>(_pos);
            upper->type = AST::Character::Type::CharacterSimple;
            upper->character = U'-';

            /* 2 discrete characters */
            result->items.push_back(std::make_pair(std::move(lower), std::shared_ptr<AST::Character>(nullptr)));
            result->items.push_back(std::make_pair(std::move(upper), std::shared_ptr<AST::Character>(nullptr)));
        }
    } while (peek() != U']');

    if (next() != U']')
        throw SyntaxError(_pos, "']' expected");

    return result;
}

std::shared_ptr<AST::SubExpr> Parser::parseSubExpr(void)
{
    char32_t ch;
    std::shared_ptr<AST::SubExpr> result = AST::Node::create<AST::SubExpr>(_pos);

    if (next() != U'(')
        throw SyntaxError(_pos, "'(' expected");

    if (!skipIf<U'?'>())
    {
        result->type = AST::SubExpr::Type::SubExprSimple;
        result->expr = parseRegExp(U')');
        _groups.push_back(result->expr);
    }
    else
    {
        switch ((ch = next()))
        {
            case U'0':
                throw SyntaxError(_pos, "Invalid group number 0");

            case U':':
            {
                result->type = AST::SubExpr::Type::SubExprNonCapture;
                result->expr = parseRegExp(U')');
                break;
            }

            case U'=':
            {
                result->type = AST::SubExpr::Type::SubExprPositiveLookahead;
                result->expr = parseRegExp(U')');
                break;
            }

            case U'!':
            {
                result->type = AST::SubExpr::Type::SubExprNegativeLookahead;
                result->expr = parseRegExp(U')');
                break;
            }

            case U'&':
            {
                std::u32string name;
                std::unordered_map<std::u32string, std::shared_ptr<AST::RegExp>>::const_iterator iter;

                while (peek() != U')')
                    name += next();

                if (name.empty())
                    throw SyntaxError(_pos, "Empty group name");

                if ((iter = _namedGroups.find(name)) == _namedGroups.end())
                    throw SyntaxError(_pos, "No such group named '" + Unicode::toString(name) + "'");

                result->expr = iter->second;
                result->type = AST::SubExpr::Type::SubExprReference;
                break;
            }

            case U'1' ... U'9':
            {
                size_t index = ch - U'0';

                while (isDigit(peek()))
                {
                    index *= 10;
                    index += next() - U'0';
                }

                if (index > _groups.size())
                    throw SyntaxError(_pos, "Invalid group number " + std::to_string(index));

                result->expr = _groups[index - 1];
                result->type = AST::SubExpr::Type::SubExprReference;
                break;
            }

            case U'<':
            {
                switch ((ch = next()))
                {
                    case U'=':
                    {
                        result->type = AST::SubExpr::Type::SubExprPositiveLookbehind;
                        result->expr = parseRegExp(U')');
                        break;
                    }

                    case U'!':
                    {
                        result->type = AST::SubExpr::Type::SubExprNegativeLookbehind;
                        result->expr = parseRegExp(U')');
                        break;
                    }

                    default:
                        throw SyntaxError(_pos, "Unknown look-behind instruction '" + Unicode::toString(ch) + "'");
                }

                break;
            }

            case U'P':
            {
                switch ((ch = next()))
                {
                    case U'=':
                    {
                        std::u32string name;
                        std::unordered_map<std::u32string, std::shared_ptr<AST::RegExp>>::const_iterator iter;

                        while (peek() != U')')
                            name += next();

                        if (name.empty())
                            throw SyntaxError(_pos, "Empty group name");

                        if ((iter = _namedGroups.find(name)) == _namedGroups.end())
                            throw SyntaxError(_pos, "No such group named '" + Unicode::toString(name) + "'");

                        result->name = iter->first;
                        result->type = AST::SubExpr::Type::SubExprMatchName;
                        break;
                    }

                    case U'<':
                    {
                        char32_t c;
                        std::u32string name;

                        while ((c = next()) != U'>')
                            name += c;

                        if (name.empty())
                            throw SyntaxError(_pos, "Empty group name");

                        if (_namedGroups.find(name) != _namedGroups.end())
                            throw SyntaxError(_pos, "Duplicated group name '" + Unicode::toString(name) + "'");

                        result->type = AST::SubExpr::Type::SubExprSimple;
                        result->expr = parseRegExp(U')');

                        _groups.push_back(result->expr);
                        _namedGroups.insert(std::make_pair(std::move(name), result->expr));
                        break;
                    }

                    default:
                        throw SyntaxError(_pos, "Unknown group naming instruction '" + Unicode::toString(ch) + "'");
                }

                break;
            }

            default:
                throw SyntaxError(_pos, "Unknown group command '" + Unicode::toString(ch) + "'");
        }
    }

    if (next() != U')')
        throw SyntaxError(_pos, "')' expected");

    return result;
}

std::shared_ptr<AST::Character> Parser::parseCharacter(void)
{
    char32_t ch;
    std::shared_ptr<AST::Character> result = AST::Node::create<AST::Character>(_pos);

    if ((ch = next()) != U'\\')
    {
        if (ch == 0)
            throw SyntaxError(_pos, "\"EOF\" when parsing characters");

        result->type = AST::Character::Type::CharacterSimple;
        result->character = ch;
    }
    else
    {
        switch ((ch = next()))
        {
            case 0:
                throw SyntaxError(_pos, "\"EOF\" when parsing escape sequence");

            case U'w': result->type = AST::Character::Type::CharacterWord  ; break;
            case U'd': result->type = AST::Character::Type::CharacterDigit ; break;
            case U's': result->type = AST::Character::Type::CharacterSpace ; break;
            case U'b': result->type = AST::Character::Type::CharacterBorder; break;

            case U'W': result->type = AST::Character::Type::CharacterNonWord  ; break;
            case U'D': result->type = AST::Character::Type::CharacterNonDigit ; break;
            case U'S': result->type = AST::Character::Type::CharacterNonSpace ; break;
            case U'B': result->type = AST::Character::Type::CharacterNonBorder; break;

            case U'n': { result->type = AST::Character::Type::CharacterSimple; result->character = U'\n'; break; }
            case U'r': { result->type = AST::Character::Type::CharacterSimple; result->character = U'\r'; break; }
            case U't': { result->type = AST::Character::Type::CharacterSimple; result->character = U'\t'; break; }

            case U'c':
            {
                switch (peek())
                {
                    case U'a' ... U'z':
                    {
                        result->type = AST::Character::Type::CharacterControl;
                        result->character = next() - U'a' + 1;
                        break;
                    }

                    case U'A' ... U'Z':
                    {
                        result->type = AST::Character::Type::CharacterControl;
                        result->character = next() - U'A' + 1;
                        break;
                    }

                    default:
                    {
                        result->type = AST::Character::Type::CharacterSimple;
                        result->character = ch;
                        break;
                    }
                }

                break;
            }

            case U'x':
            case U'u':
            case U'U':
            {
                int count = (ch == U'x') ? 2 : (ch == U'u') ? 4 : 8;
                bool requireBracket = (ch == U'x') && skipIf<U'{'>();

                result->type = AST::Character::Type::CharacterSimple;
                result->character = 0;

                for (int i = 0; i < count; i++)
                {
                    switch ((ch = next()))
                    {
                        case 0:
                            throw SyntaxError(_pos, "\"EOF\" when parsing hexadecimal characters");

                        case U'0' ... U'9':
                        {
                            result->character *= 16;
                            result->character += ch - U'0';
                            break;
                        }

                        case U'a' ... U'f':
                        {
                            result->character *= 16;
                            result->character += ch - U'a' + 10;
                            break;
                        }

                        case U'A' ... U'F':
                        {
                            result->character *= 16;
                            result->character += ch - U'A' + 10;
                            break;
                        }

                        default:
                            throw SyntaxError(_pos, "Invalid hexadecimal character '" + Unicode::toString(ch) + "'");
                    }
                }

                if (requireBracket)
                    if (next() != U'}')
                        throw SyntaxError(_pos, "'}' expected");

                break;
            }

            case U'g':
            {
                char32_t delim;
                std::u32string name;
                std::unordered_map<std::u32string, std::shared_ptr<AST::RegExp>>::const_iterator iter;

                switch (next())
                {
                    case U'{': delim = U'}'; break;
                    case U'<': delim = U'>'; break;

                    default:
                        throw SyntaxError(_pos, "Unknonwn group matching instruction '" + Unicode::toString(ch) + "'");
                }

                bool isInt = true;
                size_t index = 0;

                while ((ch = next()) != delim)
                {
                    name += ch;
                    isInt &= isDigit(ch);

                    if (isInt)
                    {
                        index *= 10;
                        index += ch - U'0';
                    }
                }

                if (name.empty())
                    throw SyntaxError(_pos, "Empty group name or index");

                if (isInt)
                {
                    if (index > 0 && index <= _groups.size())
                    {
                        if (delim == U'>')
                        {
                            result->type = AST::Character::Type::CharacterReference;
                            result->reference = _groups[index - 1];
                        }
                        else
                        {
                            result->type = AST::Character::Type::CharacterMatchIndex;
                            result->index = index;
                        }

                        break;
                    }
                }

                if ((iter = _namedGroups.find(name)) == _namedGroups.end())
                    throw SyntaxError(_pos, "No such group named '" + Unicode::toString(name) + "'");

                if (delim == U'>')
                {
                    result->type = AST::Character::Type::CharacterReference;
                    result->reference = iter->second;
                }
                else
                {
                    result->name = name;
                    result->type = AST::Character::Type::CharacterMatchName;
                }

                break;
            }

            case U'1' ... U'9':
            {
                if (!isDigit(peek()))
                {
                    if (ch - U'0' > _groups.size())
                        throw SyntaxError(_pos, "Invalid group number " + Unicode::toString(ch));

                    result->type = AST::Character::Type::CharacterMatchIndex;
                    result->index = ch - U'0';
                    break;
                }

                if (!isOctal(ch))
                {
                    result->type = AST::Character::Type::CharacterSimple;
                    result->character = ch;
                    break;
                }

                /* deliberately fall-through */
                /* break; */
            }

            case U'0':
            {
                result->type = AST::Character::Type::CharacterSimple;
                result->character = ch - U'0';

                for (int i = 0; (i < 2) && isOctal(peek()); i++)
                {
                    result->character *= 8;
                    result->character += next() - U'0';
                }

                break;
            }

            default:
            {
                result->type = AST::Character::Type::CharacterSimple;
                result->character = ch;
                break;
            }
        }
    }

    return result;
}
}