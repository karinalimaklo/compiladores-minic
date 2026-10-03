#pragma once
#include <cassert>
#include <string>
#include <vector>
#include "token.hpp"
#include "af.hpp"


class AFN : public AF {
private:
    std::vector<std::vector<int>> transicoesEps;
public:
    void adicionarEpsilon(int origem, int destino);
    int novoEstado() override;
    const std::vector<int>& getTransEpsilon(int origem)const;
    bool aceita(const std::string &entrada)const override;

};
