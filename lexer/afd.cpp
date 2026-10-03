#include <cassert>
#include <string>
#include <vector>
#include "token.hpp"
#include "afd.hpp"

int AFD:: novoEstado() {
    this->estados.push_back(Estado{});
    return (static_cast<int>(estados.size()-1));
}

bool AFD::aceita(const std::string& entrada) const {
    int q = inicial;
    for (char c : entrada) {
        q = proximo(q, c);
        if (q < 0) return false;
    }
    return estados[q].etiqueta >= 0;
}

int AFD::proximo(int q, char c) const {
    for (const Transicao& t : estados[q].transicoes)
        if (t.ini <= c && c <= t.fim) return t.destino;
    return -1;
}