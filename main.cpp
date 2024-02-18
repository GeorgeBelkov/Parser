#include "include/Parser.hpp"


int main()
{
    setlocale(LC_ALL, "RU");
    Parser parser(filename);
    parser.parse();
}