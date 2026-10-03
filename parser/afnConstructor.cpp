#include <cassert>
#include <string>
#include <vector>
#include "afn.hpp"
#include "parserAST.hpp"
#include "parserRegex.hpp"
#include "afnConstructor.hpp"

template <class ...Ts> struct overloaded : Ts... {using Ts::operator()...;};
template <class ...Ts> overloaded (Ts...) -> overloaded<Ts...>;

AFN ConstrutorAFN :: construir(const RegexNode &raiz) {
    afn = AFN{};
    Fragmento f = gerar(raiz);
    afn.setInicial (f.inicio);
    afn.setAceitacao(f.fim, 0); // nao usar
    return afn;

}

Fragmento ConstrutorAFN:: gerar(const RegexNode& no) {
    return std::visit (overloaded{
        [&](const Literal& l) {
            return gerarLiteral(l.valor);
        },
        [&](const ClasseCaractere& cc) {
            return gerarClasseCaractere(cc.intervalos);
        },
        [&](const Concat& c) {
            Fragmento a = gerar(*c.left);
            Fragmento b = gerar(*c.right);
            return gerarConcat(a, b);
        },
        [&](const Uniao& u) {
            Fragmento a = gerar(*u.left);
            Fragmento b = gerar(*u.right);
            return gerarUniao(a, b);
        },
        [&](const Star& s) {
            Fragmento a = gerar(*s.expressao);
            return gerarStar(a);

        },
    
    }, no.valor);
}

AFN ConstrutorAFN :: gerarAfnFinal(const std::vector<EntradaLexica>& tabela) {
    afn = AFN{};
    int ini = afn.novoEstado();

    for(size_t i =0; i < tabela.size(); i++) {
        Fragmento f = gerar(*tabela[i].ast);
        afn.adicionarEpsilon(ini, f.inicio);

        afn.setAceitacao(f.fim, i);

    }
    afn.setInicial(ini);
    return afn;
}

Fragmento ConstrutorAFN:: gerarLiteral(char c) {
    int i = afn.novoEstado();
    int f = afn.novoEstado();

    afn.adicionarTransicao(i, c, f);
    return Fragmento{i, f};
}
Fragmento ConstrutorAFN:: gerarClasseCaractere(const std::vector<Intervalo>& intervalos) {
    int ini = afn.novoEstado();
    int fim = afn.novoEstado();
    for(size_t i = 0; i < intervalos.size(); i++) {

        afn.adicionarTransicaoIntervalo(ini, intervalos[i].ini, intervalos[i].fim, fim);
    }
    return Fragmento{ini,fim};
}

Fragmento ConstrutorAFN:: gerarUniao(Fragmento a, Fragmento b) {
    int i = afn.novoEstado();
    int f = afn.novoEstado();
    afn.adicionarEpsilon(i, a.inicio);
    afn.adicionarEpsilon(i, b.inicio);

    afn.adicionarEpsilon(a.fim, f);
    afn.adicionarEpsilon(b.fim, f);

    return Fragmento{i, f};

}

Fragmento ConstrutorAFN:: gerarConcat(Fragmento a, Fragmento b) {
    afn.adicionarEpsilon(a.fim, b.inicio);
    return Fragmento{a.inicio, b.fim};
}

Fragmento ConstrutorAFN :: gerarStar(Fragmento a) {
    int i = afn.novoEstado();
    int f = afn.novoEstado();
    afn.adicionarEpsilon(i, f);
    afn.adicionarEpsilon(i, a.inicio);
    afn.adicionarEpsilon(a.fim, a.inicio);
    afn.adicionarEpsilon(a.fim, f);
    return Fragmento{i, f};
}