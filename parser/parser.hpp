#pragma once
#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <stdexcept>
#include <iostream>

class Parser {
private:
    std::vector tokens;
    size_t pos = 0;

    Token atual() const {
        if (pos < tokens.size()) return tokens[pos];
        return {TipoToken::FimArquivo, "", {}};
    }

    Token espiar(size_t offset = 0) const {
        if (pos + offset < tokens.size()) return tokens[pos + offset];
        return {TipoToken::FimArquivo, "", {}};
    }

    bool checar(TipoToken tipo) const {
        return atual().tipo == tipo;
    }

    Token consumir(TipoToken tipo, const std::string& mensagemErro) {
        if (checar(tipo)) {
            Token t = atual();
            pos++;
            return t;
        }
        Token errToken = atual();
        throw std::runtime_error("Erro Sintatico [Linha " + std::to_string(errToken.posicao.linha) + 
                                 ", Coluna " + std::to_string(errToken.posicao.coluna) + 
                                 "]: " + mensagemErro + ". Encontrado: '" + errToken.lexeme + "'");
    }

    bool ehTipoBasico(TipoToken tipo) const {
        return tipo == TipoToken::PalInt || tipo == TipoToken::PalBool ||
               tipo == TipoToken::PalChar || tipo == TipoToken::PalDouble;
    }

public:
    explicit Parser(std::vector tokensEntrada) : tokens(std::move(tokensEntrada)) {}

    // Ponto de entrada: Retorna a AST completa do programa!
    std::unique_ptr parsePrograma();

private:
    std::unique_ptr parseDeclaracaoFuncao();
    std::vector parseParametros();
    ParamAST parseParametro();
    std::unique_ptr parseBloco();

    std::unique_ptr parseComando();
    std::vector> parseDeclaracaoVariavel();
    std::unique_ptr parseAtribuicao();
    std::unique_ptr parseComandoIf();
    std::unique_ptr parseComandoWhile();
    std::unique_ptr parseComandoReturn();

    std::unique_ptr parseExpressao();
    std::unique_ptr parseExpressaoOr();
    std::unique_ptr parseExpressaoAnd();
    std::unique_ptr parseExpressaoIgualdade();
    std::unique_ptr parseExpressaoRelacional();
    std::unique_ptr parseExprAritmetica();
    std::unique_ptr parseTermo();
    std::unique_ptr parseFator();
    std::unique_ptr parseChamadaFuncao();
    std::vector> parseArgumentos();
};