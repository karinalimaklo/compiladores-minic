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
        std::shared_ptr<Celula> celula_temp(new Celula {valor});
        m_escopos.back()[nome] = celula_temp;
    }
}

// Procura a variável do escopo interno para o externo e atualiza seu valor
void Environment::atribuir(const std::string& nome, const Value& valor, bool is_ptr) {
    for(auto it = m_escopos.rbegin(); it != m_escopos.rend(); ++it) {
        auto enc = it->find(nome);
        if(enc != it->end()) {
            if(is_ptr) {
                std::shared_ptr<Celula> cel_temp = std::get<std::shared_ptr<Celula>> ((*enc->second).valor);
                if(cel_temp)
                    cel_temp->valor = valor;
                else
                    throw RuntimeException("Erro de Execucao: tentativa de dereferenciacao de ponteiro vazio");

            } else {
            (*enc->second).valor = valor;

            }
            return;
        }
    }
    throw RuntimeException("Erro de Execucao: Variavel '" + nome + "' nao declarada.");
}

// Procura o valor de uma variável do escopo interno para o externo
Value Environment::obter(const std::string& nome, bool isaddr, bool is_ptr) const {
    for (auto it = m_escopos.rbegin(); it != m_escopos.rend(); ++it) {
        auto enc = it->find(nome);
        if(enc != it->end()) {
            if( isaddr) {
                return enc->second;
            }
            else if (is_ptr) {
                std::shared_ptr<Celula> cel_temp = std::get<std::shared_ptr<Celula>> ((*enc->second).valor);
                if(cel_temp)
                    return cel_temp->valor; 
                else
                    throw RuntimeException("Erro de Execucao: tentativa de dereferenciacao de ponteiro vazio");
            } else {
                return (*enc->second).valor;
  
            }
        } 
    }
    throw RuntimeException("Erro de Execucao: Variavel '" + nome + "' nao foi encontrada na memoria.");
}

// Permite modificar diretamente o valor sem precisar criar diferentes operações para cada tipo de LValue.
std::shared_ptr<Celula> Environment::resolver(const std::string& nome) const {
    for (auto it = m_escopos.rbegin(); it != m_escopos.rend(); ++it) {

        auto enc = it->find(nome);
        if (enc != it->end()) {
            return enc->second;
        }
    }

    throw RuntimeException("Erro de Execucao: Variavel '" + nome + "' nao foi encontrada na memoria.");
}