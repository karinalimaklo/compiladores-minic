#include <cassert>
#include <string>
#include <vector>
#include "afn.hpp"
#include "parserAST.hpp"
#include "parserRegex.hpp"

void AFN::adicionarEpsilon(int origem, int destino) {
    assert(valido(origem) && valido(destino));
    this->transicoesEps[origem].push_back(destino);
        
}
int AFN:: novoEstado() {
    this->estados.push_back(Estado{});
    this->transicoesEps.emplace_back();
    return (static_cast<int>(estados.size())-1);
}

const std::vector<int>& AFN:: getTransEpsilon(int origem) const {
    return this->transicoesEps[origem];
}
static void fecho(const AFN& a, int q, std::vector<bool>& marcado) {
    if (marcado[q]) return;
    marcado[q] = true;
    for (int d : a.getTransEpsilon(q)) fecho(a, d, marcado);
}
bool AFN::aceita(const std::string& entrada) const {
      std::vector<bool> atual(estados.size(), false);
      fecho(*this, inicial, atual);
      for (char c : entrada) {
          std::vector<bool> prox(estados.size(), false);
          for (size_t q = 0; q < estados.size(); q++)
              if (atual[q])
                  for (const Transicao& t : estados[q].transicoes)
                      if (t.ini <= c && c <= t.fim) fecho(*this, t.destino, prox);
          atual = prox;
      }
      for (size_t q = 0; q < estados.size(); q++)
          if (atual[q] && estados[q].etiqueta >= 0) return true;
      return false;
  }