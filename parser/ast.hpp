#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../lexer/token.hpp"

//Forward Declarations de nós base para permitir referências cruzadas
class ASTNode;
class ExpressionNode;
class StatementNode;
class FunctionNode;

//Enumerações auxiliares para representar tipos de dados e operadores
enum class DataType {
    Int,
    Bool,
    Char,
    Double,
    Void
};

enum class BinaryOp {
    Add, Sub, Mul, Div, Mod,
    LessThan, LessEqual, GreaterThan, GreaterEqual,
    Equal, NotEqual,
    LogicalAnd, LogicalOr
};

enum class UnaryOp {
    LogicalNot,
    Negate
};

enum class IncDecOp {
    Increment, //++
    Decrement  //--
};

class ASTNode { //Nó base
public:
    virtual ~ASTNode() = default;
};

class ExpressionNode : public ASTNode { //Expressões
public:
    ~ExpressionNode() override = default;
};

//Literais (Ex: 42, 3.14, 'a', true)
class LiteralNode : public ExpressionNode {
public:
    DataType tipoLiteral;
    std::string valor;
    LiteralNode(DataType tipo, std::string val) : tipoLiteral(tipo), valor(std::move(val)) {}
};

//Identificador (Variáveis)
class IdentifierNode : public ExpressionNode {
public:
    std::string nome;

    explicit IdentifierNode(std::string nome) : nome(std::move(nome)) {}
};

//Operações binárias (Aritméticas, Relacionais, Igualdade, Lógicas)
class BinaryExprNode : public ExpressionNode {
public:
    BinaryOp op;
    std::unique_ptr<ExpressionNode> esquerda;
    std::unique_ptr<ExpressionNode> direita;

    BinaryExprNode(BinaryOp op, std::unique_ptr<ExpressionNode> esq, std::unique_ptr<ExpressionNode> dir)
        : op(op), esquerda(std::move(esq)), direita(std::move(dir)) {}
};

//Operações unárias (Ex: !fator, -fator)
class UnaryExprNode : public ExpressionNode {
public:
    UnaryOp op;
    std::unique_ptr<ExpressionNode> fator;

    UnaryExprNode(UnaryOp op, std::unique_ptr<ExpressionNode> fat)
        : op(op), fator(std::move(fat)) {}
};

//Incremento e decremento (Ex: id++, id--)
class IncDecExprNode : public ExpressionNode {
public:
    std::string identificador;
    IncDecOp op;

    IncDecExprNode(std::string id, IncDecOp op)
        : identificador(std::move(id)), op(op) {}
};

//Chamada de função (Ex: foo(expr1, expr2))
class FunctionCallExprNode : public ExpressionNode {
public:
    std::string nomeFuncao;
    std::vector<std::unique_ptr<ExpressionNode>> argumentos;

    FunctionCallExprNode(std::string nome, std::vector<std::unique_ptr<ExpressionNode>> args)
        : nomeFuncao(std::move(nome)), argumentos(std::move(args)) {}
};

class StatementNode : public ASTNode { //Comandos
public:
    ~StatementNode() override = default;
};

//Bloco de comandos (Ex: { comando* })
class BlockStmtNode : public StatementNode {
public:
    std::vector<std::unique_ptr<StatementNode>> comandos;

    explicit BlockStmtNode(std::vector<std::unique_ptr<StatementNode>> cmds)
        : comandos(std::move(cmds)) {}
};

//Declaração de variável única
struct VariableDeclarator {
    std::string nome;
    std::unique_ptr<ExpressionNode> inicializador; //Pode ser nullptr caso não venha com "="
};

//Declaração de variáveis (Ex: int a = 2, b;)
class VariableDeclStmtNode : public StatementNode {
public:
    DataType tipoBasico;
    std::vector<VariableDeclarator> declaradores;

    VariableDeclStmtNode(DataType tipo, std::vector<VariableDeclarator> decls)
        : tipoBasico(tipo), declaradores(std::move(decls)) {}
};

//Atribuição (Ex: x = expressao;)
class AssignmentStmtNode : public StatementNode {
public:
    std::string identificador;
    std::unique_ptr<ExpressionNode> expressao;

    AssignmentStmtNode(std::string id, std::unique_ptr<ExpressionNode> expr)
        : identificador(std::move(id)), expressao(std::move(expr)) {}
};

//Comando condicional IfElse
class IfStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condicao;
    std::unique_ptr<StatementNode> comandoThen;
    std::unique_ptr<StatementNode> comandoElse; //Pode ser nullptr se não houver 'else'

    IfStmtNode(std::unique_ptr<ExpressionNode> cond, std::unique_ptr<StatementNode> th, std::unique_ptr<StatementNode> el = nullptr)
        : condicao(std::move(cond)), comandoThen(std::move(th)), comandoElse(std::move(el)) {}
};

//Comando While
class WhileStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condicao;
    std::unique_ptr<StatementNode> comandoBody;

    WhileStmtNode(std::unique_ptr<ExpressionNode> cond, std::unique_ptr<StatementNode> body)
        : condicao(std::move(cond)), comandoBody(std::move(body)) {}
};

//Comandos de controle (Break e Continue)
class BreakStmtNode : public StatementNode {};
class ContinueStmtNode : public StatementNode {};

//Retorno
class ReturnStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> expressao; //Pode ser nullptr se for void

    explicit ReturnStmtNode(std::unique_ptr<ExpressionNode> expr = nullptr)
        : expressao(std::move(expr)) {}
};

//Comando de Expressão descartada (Ex: expressao;)
class ExpressionStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> expressao;

    explicit ExpressionStmtNode(std::unique_ptr<ExpressionNode> expr)
        : expressao(std::move(expr)) {}
};

//Parâmetro de função (Ex: int x)
struct Parameter {
    DataType tipo;
    std::string nome;
};

//Definição e declaração de funções
class FunctionNode : public ASTNode {
public:
    DataType tipoRetorno;
    std::string nome;
    std::vector<Parameter> parametros;
    std::unique_ptr<BlockStmtNode> bloco;

    FunctionNode(DataType tipo, std::string nome, std::vector<Parameter> params, std::unique_ptr<BlockStmtNode> corpo)
        : tipoRetorno(tipo), nome(std::move(nome)), parametros(std::move(params)), bloco(std::move(corpo)) {}
};

//Raiz do programa 
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionNode>> funcoes;
    std::unique_ptr<FunctionNode> funcaoMain;

    ProgramNode(std::vector<std::unique_ptr<FunctionNode>> funcs, std::unique_ptr<FunctionNode> mainFunc)
        : funcoes(std::move(funcs)), funcaoMain(std::move(mainFunc)) {}
};
