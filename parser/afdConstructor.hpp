#pragma once
#include <cstddef>
#include <map>
#include <vector>
#include "afn.hpp"
#include "afd.hpp"

class ConstrutorAFD {
    public:
        AFD construir(const AFN& afn);
    private:
        AFN afn;
        AFD afd;
        std::vector<int> visitados;
        std::map<std::vector<int>, int> mapeamento_temp;
        std::map<std::vector<int>, int> estados_afd_final;
        std::vector<std::vector<int>> conjuntos_afd;

        std::vector<int> fechamentoEpsilon(int origem);
        void movimento(int origem);
        int proximoEstado();
        int verificarEtiqueta(const std::vector<int>& conjunto);
        std::vector<int> verificaIndice(std::size_t destino_afn);
};