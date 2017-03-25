#include "Parser.h"

namespace FastRegExp
{
std::shared_ptr<AST::RegExp> Parser::parseRegExp(void)
{
    return std::shared_ptr<AST::RegExp>();
}

std::shared_ptr<AST::Section> Parser::parseSection(void)
{
    return std::shared_ptr<AST::Section>();
}

std::shared_ptr<AST::Elementry> Parser::parseElementry(void)
{
    return std::shared_ptr<AST::Elementry>();
}

std::shared_ptr<AST::Item> Parser::parseItem(void)
{
    return std::shared_ptr<AST::Item>();
}

std::shared_ptr<AST::Range> Parser::parseRange(void)
{
    return std::shared_ptr<AST::Range>();
}

std::shared_ptr<AST::SubExpr> Parser::parseSubExpr(void)
{
    return std::shared_ptr<AST::SubExpr>();
}

std::shared_ptr<AST::Character> Parser::parseCharacter(void)
{
    return std::shared_ptr<AST::Character>();
}
}