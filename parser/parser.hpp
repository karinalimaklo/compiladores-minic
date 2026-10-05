#pragma once

#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include "token.hpp"
#include "ast.hpp"

//Classe de erro para falhas de análise sintática
class ParserException : public std::runtime_error {
public:
    Posicao posicao;
    explicit ParserException(const std::string& mensagem, Posicao pos)
        : std::runtime_error(mensagem), posicao(pos) {}
};

class Parser {
public:
    //O construtor recebe a lista de tokens gerada pelo Lexer
    explicit Parser(std::vector<Token> tokens);

    //Ponto de entrada principal para processar toda a gramática
    std::unique_ptr<ProgramNode> parse();

private:
    std::vector<Token> m_tokens;
    size_t m_atual; //Indice do token que está sendo analisado no momento

    //Retorna o token atual sem consumi-lo 
    Token espiar() const;

    //Retorna o token anterior
    Token anterior() const;

    //Verifica se chegou ao fim da lista de tokens
    bool fim() const;

    //Verifica se o token atual é de um determinado tipo
    bool checar(TipoToken tipo) const;

    //Avança para o próximo token e retorna o token recém-consumido
    Token avancar();

    //Se o token atual for do tipo esperado, consome-o. Caso contrário, lança um erro sintático.
    Token consumir(TipoToken tipo, const std::string& mensagemErro);

    //Combina checar() e avancar(), onde se o token atual for de algum dos tipos listados, consome e retorna true
    bool match(const std::vector<TipoToken>& tipos);

    //Método utilitário para converter TipoToken para a enum DataType da AST
    DataType mapearTipoBasico(TipoToken tipo);

    //Estrutura global do programa
    std::unique_ptr<FunctionNode> parseDeclaracaoFuncao();
    std::unique_ptr<FunctionNode> parseFuncaoMain();
    
    //Funções e parâmetros
    std::vector<Parameter> parseParametros();
    Parameter parseParametro();
    std::vector<std::unique_ptr<ExpressionNode>> parseArgumentos();

    //Comandos e controle de fluxo
    std::unique_ptr<StatementNode> parseComando();
    std::unique_ptr<BlockStmtNode> parseBloco();
    std::unique_ptr<StatementNode> parseDeclaracaoVariavel();
    std::unique_ptr<StatementNode> parseAtribuicaoOuExpressaoStmt(); // Resolve o conflito de fator vs identificador no início da linha
    std::unique_ptr<StatementNode> parseComandoIf();
    std::unique_ptr<StatementNode> parseComandoWhile();
    std::unique_ptr<StatementNode> parseComandoBreak();
    std::unique_ptr<StatementNode> parseComandoContinue();
    std::unique_ptr<StatementNode> parseComandoReturn();

    //Expressões (Hierarquia de Precedência - Da menor para a maior)
    std::unique_ptr<ExpressionNode> parseExpressao();
    std::unique_ptr<ExpressionNode> parseExpressaoOr();
    std::unique_ptr<ExpressionNode> parseExpressaoAnd();
    std::unique_ptr<ExpressionNode> parseExpressaoIgualdade();
    std::unique_ptr<ExpressionNode> parseExpressaoRelacional();
    std::unique_ptr<ExpressionNode> parseExprAritmetica();
    std::unique_ptr<ExpressionNode> parseTermo();
    std::unique_ptr<ExpressionNode> parseFator();
};
