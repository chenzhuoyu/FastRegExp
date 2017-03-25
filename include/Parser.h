#ifndef FASTREGEXP_PARSER_H
#define FASTREGEXP_PARSER_H

#include <memory>
#include <string>

#include "AST.h"

namespace FastRegExp
{
class Parser
{
    size_t _pos;
    std::wstring _regexp;

private:
    inline wchar_t peek(void) { return _pos < _regexp.length() ? _regexp[_pos  ] : 0; }
    inline wchar_t next(void) { return _pos < _regexp.length() ? _regexp[_pos++] : 0; }

private:
    std::shared_ptr<AST::RegExp     > parseRegExp   (void);
    std::shared_ptr<AST::Section    > parseSection  (void);
    std::shared_ptr<AST::Elementry  > parseElementry(void);

private:
    std::shared_ptr<AST::Item       > parseItem     (void);
    std::shared_ptr<AST::Range      > parseRange    (void);
    std::shared_ptr<AST::SubExpr    > parseSubExpr  (void);
    std::shared_ptr<AST::Character  > parseCharacter(void);

public:
    std::shared_ptr<AST::RegExp> parse(void) { return parseRegExp(); }

};
}

#endif /* FASTREGEXP_PARSER_H */
