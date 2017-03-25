#include <locale>
#include "Parser.h"

namespace FastRegExp
{
size_t Parser::parseInt(void)
{
    if (skipIf<U'0'>())
        return 0;

    for (size_t result = 0;;)
    {
        switch (peek())
        {
            case U'0' ... U'9':
            {
                result *= 10;
                result += next() - U'0';
                break;
            }

            default:
                return result;
        }
    }
}

std::shared_ptr<AST::RegExp> Parser::parseRegExp(void)
{
    /* result `RegExp` node */
    std::shared_ptr<AST::RegExp> result = AST::Node::create<AST::RegExp>(_pos);

    /* parse each section, splitted by "|" */
    do result->sections.push_back(parseSection());
    while (skipIf<U'|'>());

    return result;
}

std::shared_ptr<AST::Section> Parser::parseSection(void)
{
    /* result `Section` node */
    std::shared_ptr<AST::Section> result = AST::Node::create<AST::Section>(_pos);

    /* parse each element */
    for (;;)
    {
        switch (peek())
        {
            case 0:
            case U'|':
                return result;

            default:
            {
                result->elements.push_back(parseElementry());
                break;
            }
        }
    }
}

std::shared_ptr<AST::Elementry> Parser::parseElementry(void)
{
    /* result `Elementry` node */
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
    // TODO: implement group parsing
    return std::shared_ptr<AST::SubExpr>();
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
                switch ((ch = next()))
                {
                    case 0:
                        throw SyntaxError(_pos, "\"EOF\" when parsing control characters");

                    case U'a' ... U'z':
                    {
                        result->type = AST::Character::Type::CharacterControl;
                        result->character = ch - U'a' + 1;
                        break;
                    }

                    case U'A' ... U'Z':
                    {
                        result->type = AST::Character::Type::CharacterControl;
                        result->character = ch - U'A' + 1;
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

            case U'0' ... U'9':
            {
                // FIXME: context-sensitive semantic here, try disambiguation and implement this feature
                throw SyntaxError(_pos, "Group numbering / octal escaping sequence not implemented due to context-sensitive sematic");
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