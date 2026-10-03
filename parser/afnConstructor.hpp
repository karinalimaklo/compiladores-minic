#pragma once
#include <cassert>
#include <string>
#include <vector>
#include "afn.hpp"
#include "gramaticaCod.hpp"
#include "parserAST.hpp"
#include "parserRegex.hpp"

class ConstrutorAFN {
    public:
        AFN construir(const RegexNode& raiz);
        AFN gerarAfnFinal(const std::vector<EntradaLexica>& tabela); 
    private:
        AFN afn;
        Fragmento gerar(const RegexNode& node);   
        Fragmento gerarLiteral(char c);
        Fragmento gerarClasseCaractere(const std::vector<Intervalo>& intervalos);
        Fragmento gerarConcat(Fragmento a, Fragmento b);
        Fragmento gerarUniao(Fragmento a, Fragmento b);
        Fragmento gerarStar(Fragmento a);

};