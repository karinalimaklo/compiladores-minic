// Gerenciador do ambiente de memória

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "value.hpp"

// Classe responsável por gerenciar a pilha de escopos (variáveis locais e globais)
class Environment {
    private:
        //pilha de escopos: o topo representa o escopo mais interno
        std::vector<std::unordered_map<std::string, std::shared_ptr<Celula>>> m_escopos;

    public:
        Environment();

        void entrarEscopo(); //cria um novo escopo de bloco '{'
        void sairEscopo(); //destrói o escopo mais interno ao sair de '}'

        void definir(const std::string& nome, const Value& valor); //declara variável no escopo atual
        void atribuir(const std::string& nome, const Value& value,bool is_ptr=false); //atualiza variável existente
        Value obter(const std::string& nome, bool isaddr = false, bool is_ptr=false) const; //busca valor da variável
        std::shared_ptr<Celula> resolver(const std::string& nome) const; //retorna a célula real de armazenamento da variável
};