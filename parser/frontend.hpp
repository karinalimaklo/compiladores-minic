#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

// Erro lancado quando o programa tem erros lexicos.
// Guarda todos os erros encontrados (o lexer nao para no primeiro).
struct ErroLexicoException : public std::runtime_error {
    std::vector<ErroLexico> erros;
    explicit ErroLexicoException(std::vector<ErroLexico> e);
};

// Front-end do compilador: recebe o texto do programa e devolve a AST nao tipada.
//
//   programa (string) -> Lexer (AFD) -> tokens -> Parser -> AST
//
// Lanca ErroLexicoException se houver caracteres invalidos e
// ErroSintatico se os tokens nao seguirem a gramatica.
std::unique_ptr<ProgramAST> gerarAST(const std::string& programa);

// Mesma coisa, reaproveitando um Lexer ja construido (evita refazer o AFD)
// e devolvendo tambem os tokens, caso queira imprimi-los.
std::unique_ptr<ProgramAST> gerarAST(const std::string& programa, const Lexer& lexer,
                                     std::vector<Token>* tokensSaida = nullptr);
