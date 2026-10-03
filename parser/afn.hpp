#pragma once
#include <cassert>
#include <string>
#include <vector>
#include "token.hpp"

struct Transicao {
    char ini;
    char fim;
    int destino;  
};

struct EstadoAFN {
    std::vector<Transicao> transicoes; 
    std::vector<int> transicoesEps;  
    int etiqueta = -1;
};


class AFN {
private:
    std::vector<EstadoAFN> estados;
    int inicial = -1;
public:
    int getInicial() const;
    const EstadoAFN& getEstado(int q) const;
    void setInicial(int ini);
    void setAceitacao(int estado, size_t indice_tabela);
    int novoEstado();

    void adicionarTransicao(int origem, char c, int destino);
    
    void adicionarTransicaoIntervalo(int origem, char simbolo_ini, char simbolo_fim, int destino);

    void adicionarEpsilon(int origem, int destino);

    bool valido(int q) const ;

    int quantidadeEstados() const;

};
struct Fragmento {
    int inicio;
    int fim;
};