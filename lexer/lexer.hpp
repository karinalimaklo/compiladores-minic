#pragma once
#include <string>
#include <vector>
#include "afd.hpp"
#include "gramaticaCod.hpp"
#include "token.hpp"

// Erro lexico: posicao e trecho que nao foi reconhecido
struct ErroLexico {
    Posicao posicao;
    std::string trecho;
};

// Lexer do MiniC++.
// O construtor monta todo o pipeline uma vez:
//   tabela de regex -> AST -> AFN geral (Thompson) -> AFD (subconjuntos).
// Depois, tokenizar() pode ser chamado quantas vezes quiser.
class Lexer {
public:
    Lexer();

    // Varre o programa com maximal munch. Devolve os tokens (sempre terminados
    // em FimArquivo) e acrescenta em `erros` os caracteres nao reconhecidos.
    std::vector<Token> tokenizar(const std::string& fonte,
                                 std::vector<ErroLexico>& erros) const;

    int estadosAFN() const { return nEstadosAfn; }
    int estadosAFD() const { return afd.quantidadeEstados(); }

private:
    std::vector<EntradaLexica> tabela;
    AFD afd;
    int nEstadosAfn = 0;
};

// Nome legivel de um tipo de token (para imprimir)
const char* nomeToken(TipoToken t);
