#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <stdexcept>
#include "../parser/ast.hpp"       
#include "symbol_table.hpp"       

//Estrutura auxiliar para guardar a assinatura de uma função global no Typechecker
struct FunctionSignature {
    DataType tipoRetorno;
    std::vector<DataType> tiposParametros;
};

//Estrutura auxiliar para guardar o esqueleto de uma Classe/Struct no Typechecker
struct ClassSignature {
    //Mapeia o nome do atributo interno para o seu respectivo tipo
    std::unordered_map<std::string, DataType> atributos;
    std::unordered_map<std::string, FunctionSignature> metodos;
};

class TypeChecker : public ASTVisitor {
private:
    //As 3 tabelas de memória do verificador
    SymbolTable m_tabVariaveis; //O ambiente Γ para variáveis locais e parâmetros
    std::unordered_map<std::string, FunctionSignature> m_tabFuncoes; //Funções globais
    std::unordered_map<std::string, ClassSignature> m_tabClasses;    //Classes/Structs globais

    //Váriaveis de estado do caminhamento
    DataType m_tipoExprAtual;    //Usada para "retornar" o tipo calculado de uma expressão
    DataType m_tipoRetornoAtual; //Guarda qual o tipo de retorno da função que está sendo visitada no momento
    int m_profundidadeLacos = 0; //Guarda quantos whiles/fors estão aninhados (0 = nenhum)
    bool sempreRetorna(StatementNode* node); //Usada para validar retorno
    void validarTipoDeclarado(const DataType& tipo, const std::string& contexto, bool permitirVoid) const; //Usada para validar Datatypes
public:
    TypeChecker() = default;

    //Ponto de entrada para iniciar a checagem a partir da raiz do programa
    void checar(ProgramNode* raiz);

    // ============================================================================
    // MÉTODOS DE VISITA HERDADOS DO ASTVISITOR
    // ============================================================================
    
    //Nível Superior e Raiz
    void visit(ProgramNode* node) override;
    void visit(FunctionNode* node) override;
    void visit(StructClassDeclStmtNode* node) override;

    //Expressões (Geram um tipo em m_tipoExprAtual)
    void visit(LiteralNode* node) override;
    void visit(IdentifierNode* node) override;
    void visit(BinaryExprNode* node) override;
    void visit(UnaryExprNode* node) override;
    void visit(IncDecExprNode* node) override;
    void visit(FunctionCallExprNode* node) override;
    void visit(MemberAccessExprNode* node) override;

    //Comandos / Statements (Apenas validam regras, não alteram m_tipoExprAtual)
    void visit(BlockStmtNode* node) override;
    void visit(VariableDeclStmtNode* node) override;
    void visit(AssignmentStmtNode* node) override;
    void visit(IfStmtNode* node) override;
    void visit(WhileStmtNode* node) override;
    void visit(BreakStmtNode* node) override;
    void visit(ContinueStmtNode* node) override;
    void visit(ReturnStmtNode* node) override;
    void visit(ExpressionStmtNode* node) override;
};
