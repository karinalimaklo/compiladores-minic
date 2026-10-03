#include <cassert>
#include <string>
#include <vector>
#include "afn.hpp"
#include "parserAST.hpp"
#include "parserRegex.hpp"

int AFN::getInicial() const{
        return this->inicial;
}
const EstadoAFN& AFN:: getEstado(int q) const {
        assert(valido(q));
        return estados[q];
}
void AFN:: setInicial(int ini) {
        assert(valido(ini));
        this->inicial = ini;
}
void AFN:: setAceitacao(int estado, size_t indice_tabela) {
        assert(valido(estado));
        this->estados[estado].etiqueta = indice_tabela;
}
int AFN:: novoEstado() {
        estados.push_back(EstadoAFN{});
        return static_cast<int>(estados.size()) - 1;
}

void AFN::adicionarTransicao(int origem, char simbolo, int destino) {
        assert(valido(origem) && valido(destino));
        estados[origem].transicoes.push_back(Transicao{simbolo, simbolo, destino});
}
void AFN::adicionarTransicaoIntervalo(int origem, char simbolo_ini, char simbolo_fim, int destino) {
    assert(valido(origem) && valido(destino));
    estados[origem].transicoes.push_back(Transicao{simbolo_ini, simbolo_fim, destino});
}
void AFN::adicionarEpsilon(int origem, int destino) {
        assert(valido(origem) && valido(destino));
        estados[origem].transicoesEps.push_back(destino);
}

bool AFN:: valido(int q) const {
        return (q >= 0 && q < static_cast<int>(estados.size()));
}

int AFN:: quantidadeEstados() const {
        return static_cast<int>(estados.size());
}
