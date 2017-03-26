#include <iostream>
#include "Parser.h"

int main(void)
{
    FastRegExp::Parser parser(R"regex((?P<asd>hello, world)(?P=asd))regex");
    std::cout << parser.parse()->toString() << std::endl;
    return 0;
}