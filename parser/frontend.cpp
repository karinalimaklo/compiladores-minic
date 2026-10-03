#include "frontend.hpp"

namespace {

std::string descreverErros(const std::vector<ErroLexico>& erros) {
    std::string msg;
    for (const ErroLexico& e : erros) {
        msg += "Erro lexico [linha " + std::to_string(e.posicao.linha) +
               ", coluna " + std::to_string(e.posicao.coluna) +
               "]: caractere inesperado '" + e.trecho + "'\n";
    }
    return msg;
}

}  // namespace

ErroLexicoException::ErroLexicoException(std::vector<ErroLexico> e)
    : std::runtime_error(descreverErros(e)), erros(std::move(e)) {}

std::unique_ptr<ProgramAST> gerarAST(const std::string& programa, const Lexer& lexer,
                                     std::vector<Token>* tokensSaida) {
    // 1. analise lexica
    std::vector<ErroLexico> erros;
    std::vector<Token> tokens = lexer.tokenizar(programa, erros);
    if (tokensSaida) *tokensSaida = tokens;
    if (!erros.empty()) {
        throw ErroLexicoException(std::move(erros));
    }

    // 2. analise sintatica
    Parser parser(std::move(tokens));
    return parser.parsePrograma();
}

std::unique_ptr<ProgramAST> gerarAST(const std::string& programa) {
    static const Lexer lexer;   // o AFD e construido uma vez so
    return gerarAST(programa, lexer);
}
