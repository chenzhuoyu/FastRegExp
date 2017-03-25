#ifndef FASTREGEXP_SYNTAXERROR_H
#define FASTREGEXP_SYNTAXERROR_H

#include <string>
#include <stdexcept>

namespace FastRegExp
{
class SyntaxError : public std::runtime_error
{
    size_t _pos;
    std::string _message;

public:
    SyntaxError(size_t pos) : SyntaxError(pos, "Syntax error") {}
    SyntaxError(size_t pos, const std::string &message) :
        std::runtime_error("Position " + std::to_string(pos) + ": " + message), _pos(pos), _message(message) {}

public:
    size_t pos(void) const { return _pos; }
    const std::string &message(void) const { return _message; }

};
}

#endif /* FASTREGEXP_SYNTAXERROR_H */
