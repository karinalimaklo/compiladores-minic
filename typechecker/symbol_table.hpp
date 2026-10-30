#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include "../parser/ast.hpp"

class SymbolTable {
private:
    //Cada mapa representa um escopo { variável -> tipo }
    //O vetor funciona como uma pilha de escopos (o último elemento é o escopo mais interno)
    std::vector<std::unordered_map<std::string, DataType>> m_escopos;

public:
    //Construtor inicializa a tabela com pelo menos o escopo global (nível 0)
    SymbolTable();

    //Cria um novo escopo interno (chamado ao entrar em um bloco '{')
    void entrarEscopo();

    //Destrói o escopo mais interno (chamado ao sair de um bloco '}')
    void sairEscopo();

    //Insere uma variável no escopo atual. Retorna false se ela já existir no mesmo escopo (redefinição inválida)
    bool inserir(const std::string& nome, const DataType& tipo);

    //Procura por uma variável do escopo mais interno para o mais externo.
    //Retorna um std::optional contendo o DataType se achar, ou std::nullopt se não declarada.
    std::optional<DataType> buscar(const std::string& nome) const;
};
