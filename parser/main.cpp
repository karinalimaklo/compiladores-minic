// main.cpp — front-end do MiniC++: programa -> tokens -> AST nao tipada
//
// Uso:
//   ./minicpp programa.mcpp            imprime a AST
//   ./minicpp programa.mcpp --tokens   imprime tambem os tokens
//   ./minicpp -e "int main() { return 0; }"   programa passado direto como string
//   ./minicpp < programa.mcpp          le da entrada padrao
//
// Codigo de saida: 0 = ok, 1 = arquivo nao encontrado,
//                  2 = erro lexico, 3 = erro sintatico

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "frontend.hpp"

int main(int argc, char* argv[]) {
    std::string arquivo;
    std::string programaDireto;
    bool temProgramaDireto = false;
    bool mostrarTokens = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--tokens") {
            mostrarTokens = true;
        } else if (arg == "-e" && i + 1 < argc) {
            programaDireto = argv[++i];
            temProgramaDireto = true;
        } else {
            arquivo = arg;
        }
    }

    // 1. ler o programa (string direta, arquivo ou entrada padrao)
    std::stringstream ss;
    if (temProgramaDireto) {
        ss << programaDireto;
    } else if (!arquivo.empty()) {
        std::ifstream arq(arquivo);
        if (!arq) {
            std::cerr << "Nao foi possivel abrir o arquivo: " << arquivo << "\n";
            return 1;
        }
        ss << arq.rdbuf();
    } else {
        ss << std::cin.rdbuf();
    }
    std::string programa = ss.str();

    // 2. montar o lexer (regex -> AFN -> AFD)
    Lexer lexer;
    std::cerr << "AFN: " << lexer.estadosAFN() << " estados | "
              << "AFD: " << lexer.estadosAFD() << " estados\n\n";

    // 3. lexer + parser
    std::vector<Token> tokens;
    try {
        std::unique_ptr<ProgramAST> ast = gerarAST(programa, lexer, &tokens);

        if (mostrarTokens) {
            std::cout << "=== TOKENS ===\n";
            for (const Token& t : tokens) {
                std::cout << t.posicao.linha << ":" << t.posicao.coluna << "\t"
                          << nomeToken(t.tipo);
                if (!t.lexeme.empty()) std::cout << "\t'" << t.lexeme << "'";
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        ast->imprimir();
    } catch (const ErroLexicoException& e) {
        std::cerr << e.what();
        return 2;
    } catch (const ErroSintatico& e) {
        std::cerr << e.what() << "\n";
        return 3;
    }
    return 0;
}
