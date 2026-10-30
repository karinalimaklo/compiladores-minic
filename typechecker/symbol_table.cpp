#include "symbol_table.hpp"

SymbolTable::SymbolTable() {
    //Inicializa a tabela criando o escopo global automaticamente
    entrarEscopo();
}

void SymbolTable::entrarEscopo() {
    m_escopos.push_back(std::unordered_map<std::string, DataType>());
}

void SymbolTable::sairEscopo() {
    //Nunca deixa a pilha de escopos vazia
    if (m_escopos.size() > 1) {
        m_escopos.pop_back();
    }
}

//Chamada quando o parser encontra uma declaração de variável
bool SymbolTable::inserir(const std::string& nome, const DataType& tipo) {
    if (m_escopos.empty()) return false;
    
    auto& escopoAtual = m_escopos.back(); //Pega a folha que está no topo da pilha
    //Verifica se a variável já existe especificamente no escopo atual
    if (escopoAtual.find(nome) != escopoAtual.end()) {
        return false; //Erro: redefinição no mesmo escopo
    }
    
    escopoAtual[nome] = tipo;
    return true;
}

//Chamado quando o usuário usa uma variável (Ex: x = 10; ou return x;)
std::optional<DataType> SymbolTable::buscar(const std::string& nome) const {
    //Varre os escopos do mais interno (fim do vetor) para o mais externo (início do vetor)
    for (auto it = m_escopos.rbegin(); it != m_escopos.rend(); ++it) {
        auto encontrado = it->find(nome); //Aponta para o par {string, Datatype}
        if (encontrado != it->end()) {
            return encontrado->second; //Retorna o tipo da variável mais próxima (Datatype)
        }
    }
    return std::nullopt; // Variável não encontrada em nenhum escopo visível
}
