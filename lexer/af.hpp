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

struct Estado {
    std::vector<Transicao> transicoes;
    int etiqueta =-1;
};
class AF {
    protected:
        std::vector<Estado> estados;
        int inicial = -1;
    public:
        virtual ~AF() = default;
        int getInicial() const;
        const Estado& getEstado(int q) const;
        void setInicial(int ini);
        void setAceitacao(int estado, int indice_tabela);
        virtual int novoEstado() = 0;

        void adicionarTransicao(int origem, char c, int destino);
        
        void adicionarTransicaoIntervalo(int origem, char simbolo_ini, char simbolo_fim, int destino);

        bool valido(int q) const ;

        int quantidadeEstados() const;

        virtual bool aceita(const std::string &entrada) const = 0;

};
struct Fragmento {
    int inicio;
    int fim;
};
    



