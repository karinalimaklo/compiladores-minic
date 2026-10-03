#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "ast.hpp"
#include "token.hpp"

// Erro sintatico com a posicao do token onde o problema foi encontrado
struct ErroSintatico : public std::runtime_error {
    Posicao posicao;
    ErroSintatico(const std::string& msg, Posicao p)
        : std::runtime_error(msg), posicao(p) {}
};

// Parser descendente recursivo (LL) para a gramatica do MiniC++.
// Recebe o vetor de tokens do lexer (terminado em FimArquivo) e devolve a AST.
class Parser {
private:
    std::vector<Token> tokens;
    size_t pos = 0;

    const Token& atual() const { return espiar(0); }

    const Token& espiar(size_t offset = 0) const {
        if (pos + offset < tokens.size()) return tokens[pos + offset];
        return tokens.back();   // o ultimo token e sempre FimArquivo
    }

    bool checar(TipoToken tipo) const {
        return atual().tipo == tipo;
    }

    [[noreturn]] void erro(const std::string& mensagem) const {
        const Token& t = atual();
        std::string encontrado = (t.tipo == TipoToken::FimArquivo) ? "fim do arquivo"
                                                                     : "'" + t.lexeme + "'";
        throw ErroSintatico("Erro sintatico [linha " + std::to_string(t.posicao.linha) +
                            ", coluna " + std::to_string(t.posicao.coluna) + "]: " +
                            mensagem + ". Encontrado: " + encontrado,
                            t.posicao);
    }

    Token consumir(TipoToken tipo, const std::string& mensagemErro) {
        if (checar(tipo)) {
            Token t = atual();
            pos++;
            return t;
        }
        erro(mensagemErro);
    }

    bool ehTipoBasico(TipoToken tipo) const {
        return tipo == TipoToken::PalInt || tipo == TipoToken::PalBool ||
               tipo == TipoToken::PalChar || tipo == TipoToken::PalDouble;
    }

public:
    explicit Parser(std::vector<Token> tokensEntrada);

    // Ponto de entrada: retorna a AST completa do programa
    std::unique_ptr<ProgramAST> parsePrograma();

private:
    std::unique_ptr<FunctionAST> parseDeclaracaoFuncao();
    std::vector<ParamAST> parseParametros();
    ParamAST parseParametro();
    std::unique_ptr<BlockStmtAST> parseBloco();

    StmtPtr parseComando();
    std::vector<StmtPtr> parseDeclaracaoVariavel();
    StmtPtr parseAtribuicao();
    StmtPtr parseComandoIf();
    StmtPtr parseComandoWhile();
    StmtPtr parseComandoReturn();

    ExprPtr parseExpressao();
    ExprPtr parseExpressaoOr();
    ExprPtr parseExpressaoAnd();
    ExprPtr parseExpressaoIgualdade();
    ExprPtr parseExpressaoRelacional();
    ExprPtr parseExprAritmetica();
    ExprPtr parseTermo();
    ExprPtr parseFator();
    ExprPtr parseChamadaFuncao();
    std::vector<ExprPtr> parseArgumentos();
};
