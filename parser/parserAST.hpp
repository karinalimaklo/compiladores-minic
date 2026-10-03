#pragma once
#include <string>
#include <vector>
#include <variant>
#include <memory>
struct Literal;
struct Concat;
struct Uniao;
struct Star;

struct RegexNode;
using RegexPtr = std::unique_ptr<RegexNode>;

struct Literal {
    char valor;
};
struct Concat {
    RegexPtr left;
    RegexPtr right;
};
struct Uniao {
    RegexPtr left;
    RegexPtr right;
};
struct Star {
    RegexPtr expressao;
};

struct Intervalo {
    char ini;
    char fim;
};

struct ClasseCaractere {
    std::vector<Intervalo> intervalos;
};

struct RegexNode {
    std::variant<Literal, Concat, Uniao, Star, ClasseCaractere> valor;
};


