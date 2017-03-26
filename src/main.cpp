#include <iostream>
#include "Parser.h"

int main(void)
{
    FastRegExp::Parser parser(R"regex((?P<123>.+)\41a)regex");
    std::cout << parser.parse()->toString() << std::endl;
    return 0;
}