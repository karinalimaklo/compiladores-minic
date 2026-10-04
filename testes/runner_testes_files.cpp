#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include "../parser/parser.hpp"
#include "../lexer/token.hpp"
#include "../lexer/lexer.hpp"

namespace fs = std::filesystem;

// Instancia o Lexer globalmente (ou dentro da main) para não reconstruir o AFD/AFN a cada arquivo
// Como o construtor do seu Lexer computa AFN->AFD, instanciá-lo uma única vez economiza muito tempo.
const Lexer& obterLexer() {
    static Lexer instanciaLexer;
    return instanciaLexer;
}

// Executa o pipeline (Lexer + Parser) em um arquivo de teste
bool testarArquivo(const std::string& caminhoArquivo, bool devePassar) {
    std::ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        std::cerr << "[ERRO] Nao foi possivel abrir o arquivo de teste: " << caminhoArquivo << "\n";
        return false;
    }

    // Lê o arquivo inteiro de teste para uma string contínua
    std::string conteudo((std::istreambuf_iterator<char>(arquivo)), std::istreambuf_iterator<char>());
    arquivo.close();

    try {
        // 1. Executa o seu Lexer Real
        std::vector<ErroLexico> errosLexicos;
        std::vector<Token> tokens = obterLexer().tokenizar(conteudo, errosLexicos);

        // Se o Lexer encontrou caracteres inválidos (erros léxicos)
        if (!errosLexicos.empty()) {
            if (!devePassar) {
                std::cout << "[ OK ] Rejeitou corretamente (Erro Lexico esperado): " << caminhoArquivo 
                          << " na Linha " << errosLexicos[0].posicao.linha << "\n";
                return true;
            } else {
                std::cout << "[FAIL] Ocorreu um Erro Lexico em um arquivo que deveria ser VALIDO: " << caminhoArquivo << "\n";
                std::cout << "       Caractere desconhecido encontrado na Linha: " << errosLexicos[0].posicao.linha 
                          << ", Coluna: " << errosLexicos[0].posicao.coluna << "\n";
                return false;
            }
        }

        // 2. Executa o seu Parser Real com os tokens gerados pelo seu Lexer
        Parser parser(std::move(tokens));
        auto ast = parser.parse(); 

        // Se o Parser aceitou o código completamente
        if (devePassar) {
            std::cout << "[ OK ] Passou (Sintaxe Valida): " << caminhoArquivo << "\n";
            return true;
        } else {
            std::cout << "[FAIL] O Parser aceitou uma sintaxe que deveria ser REJEITADA: " << caminhoArquivo << "\n";
            return false;
        }
    } 
    catch (const ParserException& e) {
        // Captura o erro sintático proposital lançado pelo parser.cpp
        if (!devePassar) {
            std::cout << "[ OK ] Rejeitou corretamente (Erro Sintatico esperado): " << caminhoArquivo << " | " << e.what() << " (Linha " << e.posicao.linha << ", Col: " << e.posicao.coluna << ")\n";
            return true;
        } else {
            std::cout << "[FAIL] O Parser rejeitou um codigo com sintaxe VALIDA: " << caminhoArquivo << "\n";
            std::cout << "       Mensagem: " << e.what() << " (Linha: " << e.posicao.linha << ", Coluna: " << e.posicao.coluna << ")\n";
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cout << "[CRASH] Ocorreu uma excecao inesperada no sistema: " << e.what() << "\n";
        return false;
    }
}

int main() {
    int testesTotais = 0;
    int testesPassados = 0;

    std::cout << "==================================================\n";
    std::cout << "    INICIANDO SUITE DE TESTES: LEXER + PARSER     \n";
    std::cout << "==================================================\n\n";

    // 1. Varre a pasta de testes que DEVEM compilar com sucesso
    std::string pastaValidos = "../testes/validos";
    if (fs::exists(pastaValidos)) {
        for (const auto& entry : fs::directory_iterator(pastaValidos)) {
            if (entry.path().extension() == ".mc" || entry.path().extension() == ".cpp") {
                testesTotais++;
                if (testarArquivo(entry.path().string(), true)) {
                    testesPassados++;
                }
            }
        }
    }

    // 2. Varre a pasta de testes que DEVEM falhar (Léxica ou Sintaticamente)
    std::string pastaInvalidos = "../testes/invalidos";
    if (fs::exists(pastaInvalidos)) {
        for (const auto& entry : fs::directory_iterator(pastaInvalidos)) {
            if (entry.path().extension() == ".mc" || entry.path().extension() == ".cpp") {
                testesTotais++;
                if (testarArquivo(entry.path().string(), false)) {
                    testesPassados++;
                }
            }
        }
    }

    std::cout << "\n==================================================\n";
    std::cout << "RESULTADO FINAL: " << testesPassados << " / " << testesTotais << " testes bem-sucedidos.\n";
    std::cout << "==================================================\n";

    return (testesPassados == testesTotais) ? 0 : 1;
}
