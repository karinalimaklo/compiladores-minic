#pragma once
#include <string>
#include <memory>
#include <vector>
#include <iostream>

// Função auxiliar para indentação visual no terminal
inline void indentar(int n) {
    for (int i = 0; i < n; ++i) std::cout << "  ";
}

// Classe base abstrata para todos os nós
struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void imprimir(int indent = 0) const = 0;
};

// ==========================================
// NÓS DE EXPRESSÃO
// ==========================================
struct ExprAST : public ASTNode {};

struct LiteralExprAST : public ExprAST {
    std::string valor;
    explicit LiteralExprAST(std::string v) : valor(std::move(v)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "Literal(" << valor << ")\n";
    }
};

struct VariableExprAST : public ExprAST {
    std::string nome;
    explicit VariableExprAST(std::string n) : nome(std::move(n)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "Var(" << nome << ")\n";
    }
};

struct BinaryExprAST : public ExprAST {
    std::string op;
    std::unique_ptr esq;
    std::unique_ptr dir;

    BinaryExprAST(std::string o, std::unique_ptr e, std::unique_ptr d)
        : op(std::move(o)), esq(std::move(e)), dir(std::move(d)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "OpBinaria(" << op << "):\n";
        if (esq) esq->imprimir(indent + 1);
        if (dir) dir->imprimir(indent + 1);
    }
};

struct UnaryExprAST : public ExprAST {
    std::string op;
    std::unique_ptr operando;

    UnaryExprAST(std::string o, std::unique_ptr opnd)
        : op(std::move(o)), operando(std::move(opnd)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "OpUnaria(" << op << "):\n";
        if (operando) operando->imprimir(indent + 1);
    }
};

struct CallExprAST : public ExprAST {
    std::string callee;
    std::vector> args;

    CallExprAST(std::string c, std::vector> a)
        : callee(std::move(c)), args(std::move(a)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "ChamadaFuncao(" << callee << "):\n";
        for (const auto& arg : args) arg->imprimir(indent + 1);
    }
};

struct IncrementExprAST : public ExprAST {
    std::string var;
    std::string op;

    IncrementExprAST(std::string v, std::string o) : var(std::move(v)), op(std::move(o)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "Incremento(" << var << op << ")\n";
    }
};

// ==========================================
// NÓS DE COMANDOS (STATEMENTS)
// ==========================================
struct StmtAST : public ASTNode {};

struct ExprStmtAST : public StmtAST {
    std::unique_ptr expr;

    explicit ExprStmtAST(std::unique_ptr e) : expr(std::move(e)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "ExprStmt:\n";
        if (expr) expr->imprimir(indent + 1);
    }
};

struct VarDeclStmtAST : public StmtAST {
    std::string tipo;
    std::string nome;
    std::unique_ptr initExpr;

    VarDeclStmtAST(std::string t, std::string n, std::unique_ptr init = nullptr)
        : tipo(std::move(t)), nome(std::move(n)), initExpr(std::move(init)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "VarDecl(" << tipo << " " << nome << ")";
        if (initExpr) {
            std::cout << " =\n";
            initExpr->imprimir(indent + 1);
        } else {
            std::cout << "\n";
        }
    }
};

struct AssignStmtAST : public StmtAST {
    std::string nomeVar;
    std::unique_ptr expr;

    AssignStmtAST(std::string n, std::unique_ptr e)
        : nomeVar(std::move(n)), expr(std::move(e)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "Atribuicao(" << nomeVar << " =):\n";
        if (expr) expr->imprimir(indent + 1);
    }
};

struct BlockStmtAST : public StmtAST {
    std::vector> comandos;

    explicit BlockStmtAST(std::vector> c) : comandos(std::move(c)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "Bloco {\n";
        for (const auto& cmd : comandos) cmd->imprimir(indent + 1);
        indentar(indent);
        std::cout << "}\n";
    }
};

struct IfStmtAST : public StmtAST {
    std::unique_ptr condicao;
    std::unique_ptr blocoThen;
    std::unique_ptr blocoElse;

    IfStmtAST(std::unique_ptr c, std::unique_ptr t, std::unique_ptr e = nullptr)
        : condicao(std::move(c)), blocoThen(std::move(t)), blocoElse(std::move(e)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "IfStmt:\n";
        indentar(indent + 1); std::cout << "[Condicao]:\n";
        condicao->imprimir(indent + 2);
        indentar(indent + 1); std::cout << "[Then]:\n";
        blocoThen->imprimir(indent + 2);
        if (blocoElse) {
            indentar(indent + 1); std::cout << "[Else]:\n";
            blocoElse->imprimir(indent + 2);
        }
    }
};

struct WhileStmtAST : public StmtAST {
    std::unique_ptr condicao;
    std::unique_ptr corpo;

    WhileStmtAST(std::unique_ptr c, std::unique_ptr b)
        : condicao(std::move(c)), corpo(std::move(b)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "WhileStmt:\n";
        indentar(indent + 1); std::cout << "[Condicao]:\n";
        condicao->imprimir(indent + 2);
        indentar(indent + 1); std::cout << "[Corpo]:\n";
        corpo->imprimir(indent + 2);
    }
};

struct ReturnStmtAST : public StmtAST {
    std::unique_ptr expr;

    explicit ReturnStmtAST(std::unique_ptr e = nullptr) : expr(std::move(e)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "ReturnStmt:\n";
        if (expr) expr->imprimir(indent + 1);
    }
};

// ==========================================
// ESTRUTURAS DE FUNÇÕES E PROGRAMA (RAÍZ)
// ==========================================
struct ParamAST {
    std::string tipo;
    std::string nome;
};

struct FunctionAST : public ASTNode {
    std::string tipoRetorno;
    std::string nome;
    std::vector parametros;
    std::unique_ptr corpo;

    FunctionAST(std::string t, std::string n, std::vector p, std::unique_ptr b)
        : tipoRetorno(std::move(t)), nome(std::move(n)), parametros(std::move(p)), corpo(std::move(b)) {}

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "FuncaoDecl: " << tipoRetorno << " " << nome << "(";
        for (size_t i = 0; i < parametros.size(); ++i) {
            std::cout << parametros[i].tipo << " " << parametros[i].nome;
            if (i + 1 < parametros.size()) std::cout << ", ";
        }
        std::cout << ")\n";
        if (corpo) corpo->imprimir(indent + 1);
    }
};

struct ProgramAST : public ASTNode {
    std::vector> funcoes;

    void imprimir(int indent = 0) const override {
        indentar(indent);
        std::cout << "=== AST DO PROGRAMA ===\n";
        for (const auto& f : funcoes) f->imprimir(indent + 1);
    }
};