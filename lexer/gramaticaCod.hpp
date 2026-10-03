#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "parserAST.hpp"
#include "token.hpp"

struct EntradaLexica {
    TipoToken tipo;
    RegexPtr ast;
};

std::vector<EntradaLexica> criarTabelaTokens();

const std::unordered_map<std::string, TipoToken>& palavrasChave();