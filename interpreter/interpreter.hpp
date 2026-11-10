#pragma once

#include <unordered_map>
#include <memory>
#include "../parser/ast.hpp"
#include "environment.hpp"
#include "value.hpp"

// Sinais de exceção usados para controle de fluxo seguro
struct ReturnException { Value valor; };
struct BreakException {};
struct ContinueException {};

class Interpreter : public ASTVisitor {
    private:
        Environment m_env; //memória dinâmica de variáveis
        std::unordered_map<std::string, FunctionNode*> m_funcoes; //registro de funções do programa
        std::unordered_map<std::string, StructClassDeclStmtNode*> m_classes; //moldes de classes/structs
        Value m_ultimoValorExpr; //acumulador do resultado da avaliação da última expressão

    public:
        Interpreter() = default;
        virtual ~Interpreter() noexcept override = default;

        // Método para iniciar a execução a partir do nó raiz do programa
        void executar(ProgramNode* raiz);

        // Métodos de visita do padrão visitor

        void visit(ProgramNode* node) override;
        void visit(FunctionNode* node) override;
        void visit(StructClassDeclStmtNode* node) override;

        void visit(LiteralNode* node) override;
        void visit(IdentifierNode* node) override;
        void visit(BinaryExprNode* node) override;
        void visit(UnaryExprNode* node) override;
        void visit(IncDecExprNode* node) override;
        void visit(FunctionCallExprNode* node) override;
        void visit(MemberAccessExprNode* node) override;
        FunctionNode* buscarMetodo(const std::string& nomeClasse, const std::string& nomeMetodo) const;

        void visit(BlockStmtNode* node) override;
        void visit(VariableDeclStmtNode* node) override;
        void visit(AssignmentStmtNode* node) override;
        void visit(IfStmtNode* node) override;
        void visit(WhileStmtNode* node) override;
        void visit(BreakStmtNode* node) override;
        void visit(ContinueStmtNode* node) override;
        void visit(ReturnStmtNode* node) override;
        void visit(ExpressionStmtNode* node) override;

    private:
        //cria uma nova instância de struct/class preenchendo os atributos com valores padrão
        std::shared_ptr<ObjectValue> criarInstanciaObjeto(const std::string& nomeClasse);

};