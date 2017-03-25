#include <iostream>
#include "Parser.h"

int main(void)
{
    FastRegExp::Parser parser(R"regex(\(?0\d{2}\)?[- ]?\d{8}|0\d{2}[- ]?\d{8,})regex");
    std::cout << parser.parse()->toString() << std::endl;
    return 0;
}