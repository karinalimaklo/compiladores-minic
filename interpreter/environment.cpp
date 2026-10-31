#include "environment.hpp"

Environment::Environment() {
    entrarEscopo(); //garante o escopo global no nível 0
}

void Environment::entrarEscopo() {
    m_escopos.push_back({});
}

void Environment::sairEscopo() {
    if(m_escopos.size() > 1) {
        m_escopos.pop_back();
    }
}

// Insere uma nova variável estritamente no escopo atual
void Environment::definir(const std::string& nome, const Value& valor) {
    if(!m_escopos.empty()) {
        m_escopos.back()[nome] = valor;
    }
}

// Procura a variável do escopo interno para o externo e atualiza seu valor
void Environment::atribuir(const std::string& nome, const Value& valor) {
    for(auto it = m_escopos.rbegin(); it != m_escopos.rend(); ++it) {
        auto enc = it->find(nome);
        if(enc != it->end()) {
            enc->second = valor;
            return;
        }
    }
    throw RuntimeException("Erro de Execucao: Variavel '" + nome + "' nao declarada.");
}

// Procura o valor de uma variável do escopo interno para o externo
Value Environment::obter(const std::string& nome) const {
    for (auto it = m_escopos.rbegin(); it != m_escopos.rend(); ++it) {
        auto enc = it->find(nome);
        if(enc != it->end()) {
            return enc->second;
        }
    }
    throw RuntimeException("Erro de Execucao: Variavel '" + nome + "' nao foi encontrada na memoria.");
}