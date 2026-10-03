#include <string>
#include <vector>
#include <variant>
#include <memory>
#include "parserAST.hpp"
#include "parserRegex.hpp"


RegexPtr parserRegex::parse () {
    RegexPtr ast = parseExpr();

    if(pos != input.size()) {
        // gerar erro
    }
    return ast;
}

RegexPtr parserRegex::parseExpr() {
    return parseUnion();
}

RegexPtr parserRegex::parseUnion() {
    RegexPtr left = parseConcat();

    while(match('|')) {
        RegexPtr right = parseConcat();

        left = std::make_unique<RegexNode>(Uniao {
            (std::move(left)),
            (std::move(right))

    });
}
    return left;
}
RegexPtr parserRegex::parseConcat() {
    RegexPtr left = parseRepetition();

    while (isAtom()) {
        RegexPtr right = parseRepetition();

        left = std::make_unique<RegexNode>(Concat {
            (std::move(left)),
            (std::move(right))

        });
    }
    return left;

}

RegexPtr parserRegex::parseRepetition() {
    RegexPtr node = parseAtom();

    while (match('*')) {
        node = std::make_unique<RegexNode>(Star {
            (std::move(node))
            });
    }
    return node;
}

RegexPtr parserRegex::parseClasse() {
    std::vector<Intervalo> intervalos_temp;

    while(pos < input.size() && peek() != ']'){
        char ini = consume();
        if (!match('-')) {
            char fim = ini;
            intervalos_temp.push_back(Intervalo{ini, fim});
            // pode gerar erro
            continue;
        }
        char fim = consume();
        intervalos_temp.push_back(Intervalo{ini, fim});
    }
    expect(']');
    RegexPtr node = std::make_unique<RegexNode>(ClasseCaractere {
            intervalos_temp
    });
    return node;
}

RegexPtr parserRegex::parseAtom() {

    if(match('(')) {
        RegexPtr node = parseExpr();

        expect(')');
        return node;
    }
    if(match('[')) {
        RegexPtr node = parseClasse();
        return node;
    }


    return parseLiteral();
}

RegexPtr parserRegex::parseLiteral() {
    if(pos >= input.size()) {
        // erro
    }
    char lit = input[pos++];

    return std::make_unique<RegexNode>(Literal{lit});
}
bool parserRegex::isAtom() {

    if(pos >= input.size()) {
        return false;
    }
    char simbolo = input[pos];
    bool resultado = (simbolo != ')'&& simbolo != '*' && simbolo != '|' && (simbolo != ']'));
    return resultado;
}

bool parserRegex::match(char c) {
    if (peek() == c) {
        pos++;
        return true;
    }
    return false;
}

char parserRegex::peek() const {
    if(pos >= input.size()) {
        return '\0';
    }
    char atual = input[pos];
    return atual;
}
char parserRegex::consume() {
    if (pos >= input.size()) {
        // retornar erro
    }
    return input[pos++];
}

void parserRegex::expect(char esperado) {
    if(!match(esperado)) {
        // gerar erro
    }
}