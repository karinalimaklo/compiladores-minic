#include <cassert>
#include <string>
#include <vector>
#include <map>
#include "afn.hpp"
#include "afd.hpp"
#include "gramaticaCod.hpp"
#include "parserAST.hpp"
#include "parserRegex.hpp"
#include "afdConstructor.hpp"
#include "util.hpp"

int ConstrutorAFD:: proximoEstado() {
    bool encontrou=false;
    size_t i=0;
    size_t j=0;
    while (!encontrou) {
        if(j == visitados.size()) {
            encontrou=true;
        }
        else if(visitados[j] == static_cast<int>(i)) {
            i++;
            continue;
        } else {
            j++;
        }

    }
    return static_cast<int>(i);
}


AFD ConstrutorAFD:: construir(const AFN& afn) {
    afd = AFD{};
    this->afn = afn;
    this->visitados.clear();
    this->mapeamento_temp.clear();
    this->estados_afd_final.clear();
    this->conjuntos_afd.clear();
    size_t estado_atual{0};
    size_t estado_atual_afd{0};
    bool terminou=false;
    while(!terminou){
        estado_atual = proximoEstado();
        std::vector<int> visit_temp = fechamentoEpsilon(estado_atual);
        size_t i=0;
        while(i < visit_temp.size()) {
            bool encontrou=false;
            size_t j=0;
            while(j < visitados.size() && !encontrou) {
                if(visit_temp[i] == visitados[j]) {
                    encontrou=true;
                } else {
                    j++;
                }
            }
            if(!encontrou) {
                visitados.push_back(visit_temp[i]);
            }
            i++;
        }
        if(static_cast<int>(visitados.size()) == afn.quantidadeEstados()) {
            terminou = true;
        }
        visitados = util::merge(visitados);
        mapeamento_temp.insert({visit_temp, estado_atual_afd});
        estado_atual_afd++;
    }
    std::vector<int> conj_temp;
    //fase 2: fechamento sobre as transições nao deterministicas que consomem caracteres
    std::vector<int> inicial = verificaIndice(afn.getInicial());
    int q0 = afd.novoEstado();
    afd.setInicial(q0);
    afd.setAceitacao(q0, verificarEtiqueta(inicial));
    this->estados_afd_final[inicial] = q0;
    conjuntos_afd.push_back(inicial);
    size_t atual = 0;
    while (atual < conjuntos_afd.size()) {
        movimento((int)atual);
        atual++;
    }
    return afd;

    
}
std::vector<int> ConstrutorAFD::fechamentoEpsilon(int origem) {
    std::vector<int> conjunto_result{origem};
    const std::vector<int>& eps = afn.getTransEpsilon(origem);   

    for (size_t j=0; j < afn.getTransEpsilon(origem).size(); j++) {
        std::vector<int> sub = fechamentoEpsilon(eps.at(j));
        conjunto_result.insert(conjunto_result.end(),sub.begin(), sub.end());
    }
    conjunto_result = util::merge(conjunto_result);
    return conjunto_result;
}
void ConstrutorAFD:: movimento(int origem) {
    std::vector<int> conjunto_afn_origem = this->conjuntos_afd[origem];
    int destino;
    std::map<std::pair<char, char>, std::vector<int>> grupos_por_intervalo; 
    size_t i =0;
    while(i < conjunto_afn_origem.size()){
        const auto& estado_atual = this->afn.getEstado(conjunto_afn_origem[i]);
        size_t j = 0;

        while(j < estado_atual.transicoes.size()) {
            grupos_por_intervalo[{estado_atual.transicoes[j].ini, estado_atual.transicoes[j].fim}].push_back(estado_atual.transicoes[j].destino);
            j++;
        }
        i++;
    }
    for (const auto& [intervalo, destinos_afn] : grupos_por_intervalo) {
        std::vector<int> conjunto_afn_dest;
        for(size_t d=0; d < destinos_afn.size(); d++) {
            std::vector<int> fecho_ini = verificaIndice(destinos_afn[d]);
            conjunto_afn_dest.insert(conjunto_afn_dest.end(), fecho_ini.begin(), fecho_ini.end());
    }
        conjunto_afn_dest = util::merge(conjunto_afn_dest);
        auto it = estados_afd_final.find(conjunto_afn_dest);
        if (it != estados_afd_final.end()) {
            destino = it->second;
        } else {
            destino = afd.novoEstado();
            afd.setAceitacao(destino, verificarEtiqueta(conjunto_afn_dest));
            estados_afd_final[conjunto_afn_dest] = destino;
            conjuntos_afd.push_back(conjunto_afn_dest);
        }
        afd.adicionarTransicaoIntervalo(origem, intervalo.first, intervalo.second, destino);
        }

}

std::vector<int> ConstrutorAFD:: verificaIndice(size_t destino_afn) {
    for(auto& [conjunto, indice]:this->mapeamento_temp) {
        for(size_t i=0; i<conjunto.size(); i++) {
            if(conjunto[i] == static_cast<int>(destino_afn)) {
                return conjunto;
            }
        }
    }
    assert(false); 
    return {};
}

int ConstrutorAFD::verificarEtiqueta(const std::vector<int>& conjunto) {
    int menor = -1;
    for (size_t q=0; q<conjunto.size(); q++) {
        int e = afn.getEstado(conjunto.at(q)).etiqueta;
        if (e >= 0 && (menor < 0 || e < menor)){
            menor = e;
        }
    }
    return menor;
}