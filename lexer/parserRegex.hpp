#pragma once
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include "parserAST.hpp"
class parserRegex {
    private:
        std::size_t pos;
        std::string input;

    public:
        parserRegex(std::string input)
    : pos(0), input(input)
{
}
        RegexPtr parse();
        RegexPtr parseExpr();
        RegexPtr parseUnion();
        RegexPtr parseConcat();
        RegexPtr parseRepetition();
        RegexPtr parseClasse();
        RegexPtr parseAtom();
        RegexPtr parseLiteral();

        bool match(char c);

        bool isAtom();

        char peek() const ;

        char consume();

        void expect(char c);


};