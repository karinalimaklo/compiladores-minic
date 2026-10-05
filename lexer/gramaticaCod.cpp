#include "gramaticaCod.hpp"

#include <cassert>
#include "parserRegex.hpp"

namespace {


RegexPtr regex(const std::string& texto) {
    parserRegex parser(texto);
    return parser.parse();
}


RegexPtr textoFixo(const std::string& texto) {
    assert(!texto.empty());
    RegexPtr ast = std::make_unique<RegexNode>(RegexNode{ Literal{texto[0]} });
    for (std::size_t i = 1; i < texto.size(); ++i) {
        RegexPtr prox = std::make_unique<RegexNode>(RegexNode{ Literal{texto[i]} });
        ast = std::make_unique<RegexNode>(RegexNode{ Concat{ std::move(ast), std::move(prox) } });
    }
    return ast;
}

// digito ::= "0" | ... | "9"
const std::string DIGITO = "0-9";

// digito_nao_nulo ::= "1" | ... | "9"
const std::string NAO_NULO = "1-9";

// letra ::= "a" | ... | "z" | "A" | ... | "Z"
const std::string LETRA = "a-zA-Z";

// caractere ::= letra | digito | " "
const std::string CARACTERE = "[" + LETRA  + DIGITO + " ]";

}  

std::vector<EntradaLexica> criarTabelaTokens() {
    std::vector<EntradaLexica> tabela;

    // numero_inteiro ::= "0" | digito_nao_nulo (digito)*
    tabela.push_back({TipoToken::IntLit,
        regex("0|[" + NAO_NULO + "][" + DIGITO + "]*")});

    // numero_real ::= digito_nao_nulo digito* "." digito+ | "0" "." digito+
    tabela.push_back({TipoToken::DoubleLit,
        regex("[" + NAO_NULO + "][" + DIGITO + "]*.[" + DIGITO + "][" + DIGITO + "]*"
              "|0.[" + DIGITO + "][" + DIGITO + "]*")});

    // identificador ::= (letra) (letra | digito | "_")*
    tabela.push_back({TipoToken::Ident,
        regex("["+ LETRA+ "]" + "[" + LETRA  + DIGITO + "_]*")});

    // literal_char ::= "'" caractere "'"
    tabela.push_back({TipoToken::CharLit,
        regex("'" + CARACTERE + "'")});

    // literal_string ::= '"' caractere* '"'
    tabela.push_back({TipoToken::StringLit,
        regex("\"" + CARACTERE + "*\"")});

    // op_aditivo, op_multiplicativo
    tabela.push_back({TipoToken::Plus,         textoFixo("+")});
    tabela.push_back({TipoToken::Min,          textoFixo("-")});
    tabela.push_back({TipoToken::Star,         textoFixo("*")});
    tabela.push_back({TipoToken::Div,          textoFixo("/")});
    tabela.push_back({TipoToken::Percent,      textoFixo("%")});

    // op_atribuicao
    tabela.push_back({TipoToken::Assign,       textoFixo("=")});
    tabela.push_back({TipoToken::PlusAssign,   textoFixo("+=")});
    tabela.push_back({TipoToken::MinAssign,    textoFixo("-=")});

    // op_igualdade, op_relacional
    tabela.push_back({TipoToken::Eq,           textoFixo("==")});
    tabela.push_back({TipoToken::NotEq,        textoFixo("!=")});
    tabela.push_back({TipoToken::Gt,           textoFixo(">")});
    tabela.push_back({TipoToken::Ge,           textoFixo(">=")});
    tabela.push_back({TipoToken::Lt,           textoFixo("<")});
    tabela.push_back({TipoToken::Le,           textoFixo("<=")});

    // op_logico_and, op_logico_or, op_unario
    tabela.push_back({TipoToken::AndAnd,       textoFixo("&&")});
    tabela.push_back({TipoToken::OrOr,         textoFixo("||")});
    tabela.push_back({TipoToken::Not,          textoFixo("!")});

    // incremento
    tabela.push_back({TipoToken::PlusPLus,     textoFixo("++")});
    tabela.push_back({TipoToken::MinMin,       textoFixo("--")});

    // expressao_ternaria
    tabela.push_back({TipoToken::Interrogacao, textoFixo("?")});
    tabela.push_back({TipoToken::DoisPontos,   textoFixo(":")});

    // pontuacao
    tabela.push_back({TipoToken::ColEsquerda,  textoFixo("(")});
    tabela.push_back({TipoToken::ColDireita,   textoFixo(")")});
    tabela.push_back({TipoToken::ChavesEsq,    textoFixo("{")});
    tabela.push_back({TipoToken::ChavesDir,    textoFixo("}")});
    tabela.push_back({TipoToken::ColchEsq,     textoFixo("[")});
    tabela.push_back({TipoToken::ColchDir,     textoFixo("]")});
    tabela.push_back({TipoToken::Virg,         textoFixo(",")});
    tabela.push_back({TipoToken::PontoVirg,    textoFixo(";")});

    return tabela;
}

const std::unordered_map<std::string, TipoToken>& palavrasChave() {
    static const std::unordered_map<std::string, TipoToken> tabela = {
        // tipo_basico, tipo_retorno
        {"int", TipoToken::PalInt},       {"char", TipoToken::PalChar},
        {"double", TipoToken::PalDouble}, {"bool", TipoToken::PalBool},
        {"string", TipoToken::PalString}, {"void", TipoToken::PalVoid},
        // literal_bool
        {"true", TipoToken::PalTrue},     {"false", TipoToken::PalFalse},
        // comandos
        {"if", TipoToken::PalIf},         {"else", TipoToken::PalElse},
        {"while", TipoToken::PalWhile},   {"for", TipoToken::PalFor},
        {"break", TipoToken::PalBreak},   {"continue", TipoToken::PalContinue},
        {"return", TipoToken::PalRet},
    };
    return tabela;
}