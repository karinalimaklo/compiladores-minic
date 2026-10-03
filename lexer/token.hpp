#pragma once
#include <string>

struct Posicao {
    int linha = 1;
    int coluna = 1;
};

enum class TipoToken {
    // Literais
    IntLit, DoubleLit, CharLit, StringLit, Ident,
    // Palavras-chave
    PalInt, PalChar, PalDouble, PalBool, PalString, PalVoid,
    PalTrue, PalFalse,
    PalIf, PalElse, PalWhile, PalFor, PalBreak, PalContinue, PalRet,
    PalCin, PalCout,
    // Operadores
    Plus, Min, Star, Div, Percent,
    Assign, PlusAssign, MinAssign,
    Eq, NotEq, Gt, Ge, Lt, Le,
    AndAnd, OrOr, Not,
    PlusPLus, MinMin,
    Shl, Shr,               // << (cout) e >> (cin)
    Interrogacao, DoisPontos,  // ? e : (expressao ternaria)
    // Pontuacao
    ColEsquerda, ColDireita, ChavesEsq, ChavesDir, ColchEsq, ColchDir,
    Virg, PontoVirg,
    FimArquivo
};

struct Token {
    TipoToken tipo;
    std::string lexeme;
    Posicao posicao;
};