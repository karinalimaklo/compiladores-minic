#pragma once
#include <cassert>
#include <string>
#include <vector>
#include "token.hpp"
#include "af.hpp"

class AFD :public  AF {
    public:
        int novoEstado() override;
        int proximo(int q, char c) const;
        bool aceita(const std::string &entrada) const override;

};