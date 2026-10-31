#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../lexer/token.hpp"

//Forward Declarations
class ASTNode;
class ExpressionNode;
class StatementNode;
class FunctionNode;
class LiteralNode;
class IdentifierNode;
class BinaryExprNode;
class UnaryExprNode;
class IncDecExprNode;
class FunctionCallExprNode;
class BlockStmtNode;
class VariableDeclStmtNode;
class AssignmentStmtNode;
class IfStmtNode;
class WhileStmtNode;
class BreakStmtNode;
class ContinueStmtNode;
class ReturnStmtNode;
class ExpressionStmtNode;
class ProgramNode;
class StructClassDeclStmtNode;
class MemberAccessExprNode;

//Enumerações auxiliares para representar tipos de dados e operadores
enum class DataTypeKind {
    Int,
    Bool,
    Char,
    Double,
    Void,
    Pointer,
    Class
};

struct DataType { //struct para saber qual o tipo de dado, o nível do ponteiro e o nome da classe se for uma
    DataTypeKind kind;
    std::string name; //Preenchido se for definido pelo usuário 
    int pointerLevel = 0; //0 = comum, 1 = *, 2 = **, etc.
};

enum class BinaryOp {
    Add, Sub, Mul, Div, Mod,
    LessThan, LessEqual, GreaterThan, GreaterEqual,
    Equal, NotEqual,
    LogicalAnd, LogicalOr
};

enum class UnaryOp {
    LogicalNot,
    Negate,
    AddressOf, //&x
    Deref //*p
};

enum class IncDecOp {
    Increment, //++
    Decrement  //--
};


//Classe base abstrata do visitor
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    //Métodos de visita para expressões
    virtual void visit(LiteralNode* node) = 0;
    virtual void visit(IdentifierNode* node) = 0;
    virtual void visit(BinaryExprNode* node) = 0;
    virtual void visit(UnaryExprNode* node) = 0;
    virtual void visit(IncDecExprNode* node) = 0;
    virtual void visit(FunctionCallExprNode* node) = 0;

    //Métodos de visita para comandos
    virtual void visit(BlockStmtNode* node) = 0;
    virtual void visit(VariableDeclStmtNode* node) = 0;
    virtual void visit(AssignmentStmtNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(WhileStmtNode* node) = 0;
    virtual void visit(BreakStmtNode* node) = 0;
    virtual void visit(ContinueStmtNode* node) = 0;
    virtual void visit(ReturnStmtNode* node) = 0;
    virtual void visit(ExpressionStmtNode* node) = 0;

    //Métodos de visita para estruturas de nível superior
    virtual void visit(FunctionNode* node) = 0;
    virtual void visit(ProgramNode* node) = 0;

    //Métodos para orientação a objetos
    virtual void visit(StructClassDeclStmtNode* node) = 0;
    virtual void visit(MemberAccessExprNode* node) = 0;
};

class ASTNode { //Nó base
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor* visitor) = 0; 
};

class ExpressionNode : public ASTNode { //Expressões
public:
    ~ExpressionNode() override = default;
    //herdado: virtual void accept(ASTVisitor* visitor) = 0;
};

class StatementNode : public ASTNode { // Comandos
public:
    ~StatementNode() override = default;
    //herdado: virtual void accept(ASTVisitor* visitor) = 0;
};

//Literais (Ex: 42, 3.14, 'a', true)
class LiteralNode : public ExpressionNode {
public:
    DataType tipoLiteral;
    std::string valor;
    LiteralNode(DataType tipo, std::string val) : tipoLiteral(tipo), valor(std::move(val)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Identificador (Variáveis)
class IdentifierNode : public ExpressionNode {
public:
    std::string nome;
    explicit IdentifierNode(std::string nome) : nome(std::move(nome)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Operações binárias (Aritméticas, Relacionais, Igualdade, Lógicas)
class BinaryExprNode : public ExpressionNode {
public:
    BinaryOp op;
    std::unique_ptr<ExpressionNode> esquerda;
    std::unique_ptr<ExpressionNode> direita;

    BinaryExprNode(BinaryOp op, std::unique_ptr<ExpressionNode> esq, std::unique_ptr<ExpressionNode> dir)
        : op(op), esquerda(std::move(esq)), direita(std::move(dir)) {}
    
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Operações unárias (Ex: !fator, -fator)
class UnaryExprNode : public ExpressionNode {
public:
    UnaryOp op;
    std::unique_ptr<ExpressionNode> fator;

    UnaryExprNode(UnaryOp op, std::unique_ptr<ExpressionNode> fat)
        : op(op), fator(std::move(fat)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Incremento e decremento (Ex: id++, id--)
class IncDecExprNode : public ExpressionNode {
public:
    std::string identificador;
    IncDecOp op;

    IncDecExprNode(std::string id, IncDecOp op)
        : identificador(std::move(id)), op(op) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Chamada de função (Ex: foo(expr1, expr2))
class FunctionCallExprNode : public ExpressionNode {
public:
    std::string nomeFuncao;
    std::vector<std::unique_ptr<ExpressionNode>> argumentos;

    FunctionCallExprNode(std::string nome, std::vector<std::unique_ptr<ExpressionNode>> args)
        : nomeFuncao(std::move(nome)), argumentos(std::move(args)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Acesso a membro de Classe/Struct (Ex: objeto.campo ou ponteiro->campo)
class MemberAccessExprNode : public ExpressionNode {
public:
    std::unique_ptr<ExpressionNode> objeto;
    std::string nomeMembro;
    bool isArrow; //true se for '->', false se for '.'

    MemberAccessExprNode(std::unique_ptr<ExpressionNode> obj, std::string membro, bool arrow)
        : objeto(std::move(obj)), nomeMembro(std::move(membro)), isArrow(arrow) {}

    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

//Atributo interno de uma Struct/Classe
struct FieldDeclarator {
    DataType tipo;
    std::string nome;
};

//Declaração de Struct ou Classe (Ex: struct Nome { campos };)
class StructClassDeclStmtNode : public StatementNode {
public:
    std::string nome;
    std::vector<FieldDeclarator> campos;
    bool isClass; //true se foi declarada com 'class', false se 'struct'

    StructClassDeclStmtNode(std::string nome, std::vector<FieldDeclarator> campos, bool isClass)
        : nome(std::move(nome)), campos(std::move(campos)), isClass(isClass) {}

    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

//Bloco de comandos (Ex: { comando* })
class BlockStmtNode : public StatementNode {
public:
    std::vector<std::unique_ptr<StatementNode>> comandos;

    explicit BlockStmtNode(std::vector<std::unique_ptr<StatementNode>> cmds)
        : comandos(std::move(cmds)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
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
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Atribuição (Ex: x = expressao;)
class AssignmentStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> esquerda; //Aceita qualquer expressão válida à esquerda do '='
    std::unique_ptr<ExpressionNode> direita;  

    AssignmentStmtNode(std::unique_ptr<ExpressionNode> esq, std::unique_ptr<ExpressionNode> dir)
        : esquerda(std::move(esq)), direita(std::move(dir)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Comando condicional IfElse
class IfStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condicao;
    std::unique_ptr<StatementNode> comandoThen;
    std::unique_ptr<StatementNode> comandoElse; //Pode ser nullptr se não houver 'else'

    IfStmtNode(std::unique_ptr<ExpressionNode> cond, std::unique_ptr<StatementNode> th, std::unique_ptr<StatementNode> el = nullptr)
        : condicao(std::move(cond)), comandoThen(std::move(th)), comandoElse(std::move(el)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Comando While
class WhileStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> condicao;
    std::unique_ptr<StatementNode> comandoBody;

    WhileStmtNode(std::unique_ptr<ExpressionNode> cond, std::unique_ptr<StatementNode> body)
        : condicao(std::move(cond)), comandoBody(std::move(body)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Comandos de controle (Break e Continue)
class BreakStmtNode : public StatementNode {
public:

void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};
class ContinueStmtNode : public StatementNode {
public:

void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Retorno
class ReturnStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> expressao; //Pode ser nullptr se for void

    explicit ReturnStmtNode(std::unique_ptr<ExpressionNode> expr = nullptr)
        : expressao(std::move(expr)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Comando de Expressão descartada (Ex: expressao;)
class ExpressionStmtNode : public StatementNode {
public:
    std::unique_ptr<ExpressionNode> expressao;

    explicit ExpressionStmtNode(std::unique_ptr<ExpressionNode> expr)
        : expressao(std::move(expr)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
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
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};

//Raiz do programa 
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<StatementNode>> classes_structs; 
    std::vector<std::unique_ptr<FunctionNode>> funcoes;
    std::unique_ptr<FunctionNode> funcaoMain;

    ProgramNode(std::vector<std::unique_ptr<StatementNode>> classes,
                std::vector<std::unique_ptr<FunctionNode>> funcs, 
                std::unique_ptr<FunctionNode> mainFunc)
        : classes_structs(std::move(classes)), funcoes(std::move(funcs)), funcaoMain(std::move(mainFunc)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); } 
};
