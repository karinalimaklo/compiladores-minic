#include <cassert>
#include <string>
#include <vector>
#include "token.hpp"
#include "af.hpp"

int AF:: getInicial() const {
    return this->inicial;
 }
const Estado& AF::getEstado(int q) const {
    assert(valido(q));
    return this->estados[q];
}
void AF::setInicial(int ini) {
    assert(valido(ini));
    this->inicial = ini;
}
void AF::setAceitacao(int estado, int indice_tabela) {
    assert(valido(estado));
    this->estados[estado].etiqueta = indice_tabela;
}
void AF::adicionarTransicao(int origem, char c, int destino) {
    assert(valido(origem));
    assert(valido(destino));
    this->estados[origem].transicoes.push_back(Transicao{c, c, destino});

}
    
void AF::adicionarTransicaoIntervalo(int origem, char simbolo_ini, char simbolo_fim, int destino){
    assert(valido(origem));
    assert(valido(destino));
    this->estados[origem].transicoes.push_back(Transicao{simbolo_ini, simbolo_fim, destino});
}

bool AF::valido(int q) const {
    bool resultado = (q >= 0 && (q < static_cast<int>(estados.size())));
    return resultado;
}

int AF::quantidadeEstados() const{
    return static_cast<int>(estados.size());
}
