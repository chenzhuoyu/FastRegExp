#ifndef FASTREGEXP_PARSER_H
#define FASTREGEXP_PARSER_H

#include <memory>
#include <string>

#include "AST.h"
#include "Unicode.h"
#include "SyntaxError.h"

namespace FastRegExp
{
class Parser
{
    size_t _pos;
    std::u32string _regexp;

public:
    explicit Parser(const std::string &regexp) : _pos(0), _regexp(Unicode::toUnicode(regexp)) {}

private:
    size_t parseInt(void);

private:
    inline char32_t peek(void) { return _pos < _regexp.length() ? _regexp[_pos  ] : 0; }
    inline char32_t next(void) { return _pos < _regexp.length() ? _regexp[_pos++] : 0; }

private:
    template <char32_t ch>
    inline bool skipIf(void)
    {
        if (peek() != ch)
            return false;

        next();
        return true;
    }

private:
    std::shared_ptr<AST::RegExp     > parseRegExp   (void);
    std::shared_ptr<AST::Section    > parseSection  (void);
    std::shared_ptr<AST::Elementry  > parseElementry(void);

private:
    std::shared_ptr<AST::Range      > parseRange    (void);
    std::shared_ptr<AST::SubExpr    > parseSubExpr  (void);
    std::shared_ptr<AST::Character  > parseCharacter(void);

public:
    std::shared_ptr<AST::Node> parse(void) { return parseRegExp(); }

};
}

#endif /* FASTREGEXP_PARSER_H */
