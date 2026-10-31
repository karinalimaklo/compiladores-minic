#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include "../parser/parser.hpp"
#include "../lexer/token.hpp"
#include "../lexer/lexer.hpp"
#include "../typechecker/typechecker.hpp"

namespace fs = std::filesystem;

// Instancia o Lexer globalmente (ou dentro da main) para não reconstruir o AFD/AFN a cada arquivo
// Como o construtor do seu Lexer computa AFN->AFD, instanciá-lo uma única vez economiza muito tempo.
const Lexer& obterLexer() {
    static Lexer instanciaLexer;
    return instanciaLexer;
}
enum class Expectativa { Valido, ErroSintatico, ErroSemantico };

// Executa o pipeline (Lexer + Parser) em um arquivo de teste
bool testarArquivo(const std::string& caminhoArquivo, Expectativa expectativa) {
    std::ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        std::cerr << "[ERRO] Nao foi possivel abrir o arquivo de teste: " << caminhoArquivo << "\n";
        return false;
    }
    // Lê o arquivo inteiro de teste para uma string contínua
    std::string conteudo((std::istreambuf_iterator<char>(arquivo)), std::istreambuf_iterator<char>());
    arquivo.close();

    try {
        // 1. PIPELINE - LEXER
        std::vector<ErroLexico> errosLexicos;
        std::vector<Token> tokens = obterLexer().tokenizar(conteudo, errosLexicos);

        if (!errosLexicos.empty()) {
            // Se esperávamos algum erro de sintaxe/léxico, aceita. Mas se esperávamos erro SEMÂNTICO, falhou (o erro devia ser depois)
            if (expectativa == Expectativa::ErroSintatico) {
                std::cout << "[ OK ] Rejeitou corretamente (Erro Lexico/Sintatico esperado): " << caminhoArquivo << "\n";
                return true;
            } else {
                std::cout << "[FAIL] Ocorreu um Erro Lexico inesperado em: " << caminhoArquivo << "\n";
                return false;
            }
        }

        // 2. PIPELINE - PARSER
        Parser parser(std::move(tokens));
        auto ast = parser.parse(); // Se a sintaxe for inválida, joga ParserException aqui

        if (expectativa == Expectativa::ErroSintatico) {
            std::cout << "[FAIL] O Parser aceitou uma sintaxe que deveria ser REJEITADA: " << caminhoArquivo << "\n";
            return false;
        }

        // 3. PIPELINE - TYPECHECKER (A nova etapa do pipeline!)
        TypeChecker checker;
        checker.checar(ast.get()); // Se houver erro de tipo/escopo, joga std::runtime_error aqui

        // Se chegou até aqui sem nenhuma exceção, o código está 100% limpo
        if (expectativa == Expectativa::Valido) {
            std::cout << "[ OK ] Passou com Sucesso (Codigo Totalmente Valido): " << caminhoArquivo << "\n";
            return true;
        } else {
            std::cout << "[FAIL] O Typechecker aceitou um erro semantico que deveria ser REJEITADO: " << caminhoArquivo << "\n";
            return false;
        }
    } 
    catch (const ParserException& e) {
        if (expectativa == Expectativa::ErroSintatico) {
            std::cout << "[ OK ] Rejeitou corretamente (Erro Sintatico esperado): " << caminhoArquivo << " | " << e.what() << "\n";
            return true;
        } else {
            std::cout << "[FAIL] O Parser rejeitou um codigo que deveria passar na sintaxe: " << caminhoArquivo << "\n";
            std::cout << "       Mensagem: " << e.what() << "\n";
            return false;
        }
    }
    catch (const std::runtime_error& e) {
        // Captura os erros semânticos intencionais lançados pelo seu typechecker.cpp
        if (expectativa == Expectativa::ErroSemantico) {
            std::cout << "[ OK ] Rejeitou corretamente (Erro Semantico esperado): " << caminhoArquivo << " | " << e.what() << "\n";
            return true;
        } else {
            std::cout << "[FAIL] O Typechecker rejeitou um codigo semanticamente VALIDO: " << caminhoArquivo << "\n";
            std::cout << "       Mensagem: " << e.what() << "\n";
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
    std::cout << "  SUITE DE TESTES INTEGRADOS: LEXER+PARSER+SEMANTICO \n";
    std::cout << "==================================================\n\n";

    // 1. Varre a pasta de testes que DEVEM compilar com sucesso total
    std::string pastaValidos = "../testes/validos";
    if (fs::exists(pastaValidos)) {
        std::cout << "--- Testando Codigos Validos ---\n";
        for (const auto& entry : fs::directory_iterator(pastaValidos)) {
            if (entry.path().extension() == ".mc" || entry.path().extension() == ".cpp") {
                testesTotais++;
                if (testarArquivo(entry.path().string(), Expectativa::Valido)) {
                    testesPassados++;
                }
            }
        }
        std::cout << "\n";
    }

    // 2. Varre a pasta de testes que DEVEM falhar na gramatica (Lexer/Parser)
    std::string pastaInvalidosSintaxe = "../testes/invalidos_sintaxe";
    if (fs::exists(pastaInvalidosSintaxe)) {
        std::cout << "--- Testando Erros Sintaticos Esperados ---\n";
        for (const auto& entry : fs::directory_iterator(pastaInvalidosSintaxe)) {
            if (entry.path().extension() == ".mc" || entry.path().extension() == ".cpp") {
                testesTotais++;
                if (testarArquivo(entry.path().string(), Expectativa::ErroSintatico)) {
                    testesPassados++;
                }
            }
        }
        std::cout << "\n";
    }

    // 3. Varre a pasta de testes que DEVEM falhar nas regras de tipo/escopo (Typechecker)
    std::string pastaInvalidosSemantica = "../testes/invalidos_semantica";
    if (fs::exists(pastaInvalidosSemantica)) {
        std::cout << "--- Testando Erros Semanticos Esperados ---\n";
        for (const auto& entry : fs::directory_iterator(pastaInvalidosSemantica)) {
            if (entry.path().extension() == ".mc" || entry.path().extension() == ".cpp") {
                testesTotais++;
                if (testarArquivo(entry.path().string(), Expectativa::ErroSemantico)) {
                    testesPassados++;
                }
            }
        }
        std::cout << "\n";
    }

    std::cout << "==================================================\n";
    std::cout << "RESULTADO FINAL: " << testesPassados << " / " << testesTotais << " testes bem-sucedidos.\n";
    std::cout << "==================================================\n";

    return (testesPassados == testesTotais) ? 0 : 1;
}
