#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include "../parser/parser.hpp"
#include "../lexer/token.hpp"
#include "../lexer/lexer.hpp"
#include "../typechecker/typechecker.hpp"
#include "../interpreter/interpreter.hpp"

namespace fs = std::filesystem;

// Instancia o Lexer globalmente (ou dentro da main) para não reconstruir o AFD/AFN a cada arquivo
const Lexer& obterLexer() {
    static Lexer instanciaLexer;
    return instanciaLexer;
}
enum class Expectativa { Valido, ErroSintatico, ErroSemantico, ErroRuntime };

// Executa o pipeline completo (Lexer + Parser + Typechecker + Interpreter)
bool testarArquivo(const std::string& caminhoArquivo, Expectativa expectativa) {
    std::ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        std::cerr << "[ERRO] Nao foi possivel abrir o arquivo de teste: " << caminhoArquivo << "\n";
        return false;
    }

    std::string conteudo((std::istreambuf_iterator<char>(arquivo)), std::istreambuf_iterator<char>());
    arquivo.close();

    try {
        // 1. PIPELINE - LEXER
        std::vector<ErroLexico> errosLexicos;
        std::vector<Token> tokens = obterLexer().tokenizar(conteudo, errosLexicos);

        if (!errosLexicos.empty()) {
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
        auto ast = parser.parse(); 

        if (expectativa == Expectativa::ErroSintatico) {
            std::cout << "[FAIL] O Parser aceitou uma sintaxe que deveria ser REJEITADA: " << caminhoArquivo << "\n";
            return false;
        }

        // 3. PIPELINE - TYPECHECKER
        TypeChecker checker;
        checker.checar(ast.get()); 

        if (expectativa == Expectativa::ErroSemantico) {
            std::cout << "[FAIL] O Typechecker aceitou um erro semantico que deveria ser REJEITADO: " << caminhoArquivo << "\n";
            return false;
        }

        // 4. PIPELINE - INTERPRETER 
        if (expectativa == Expectativa::Valido || expectativa == Expectativa::ErroRuntime) {
            Interpreter interpretador;
            
            interpretador.executar(ast.get()); 
            
            std::cout << "[ OK ] Executou com Sucesso (Codigo Valido e Rodando): " << caminhoArquivo << "\n";
            return true;
        }

    } 
    catch (const ParserException& e) {
        if (expectativa == Expectativa::ErroSintatico) {
            std::cout << "[ OK ] Rejeitou corretamente (Erro Sintatico esperado): " << caminhoArquivo << " | " << e.what() << "\n";
            return true;
        } else {
            std::cout << "[FAIL] O Parser rejeitou um codigo que deveria passar na sintaxe: " << caminhoArquivo << "\n";
            return false;
        }
    }
    catch (const RuntimeException& e) {
        if (expectativa == Expectativa::ErroRuntime) {
            std::cout << "[ OK ] Rejeitou corretamente (Erro em Tempo de Execucao esperado): " << caminhoArquivo << " | " << e.what() << "\n";
            return true;
        } else {
            std::cout << "[FAIL] O Interpretador quebrou inesperadamente durante a execucao: " << caminhoArquivo << " | " << e.what() << "\n";
            return false;
        }
    }
    catch (const std::runtime_error& e) {
        // Captura os erros semânticos lançados pelo typechecker.cpp
        if (expectativa == Expectativa::ErroSemantico) {
            std::cout << "[ OK ] Rejeitou corretamente (Erro Semantico esperado): " << caminhoArquivo << " | " << e.what() << "\n";
            return true;
        } else {
            std::cout << "[FAIL] O Typechecker rejeitou um codigo semanticamente VALIDO: " << caminhoArquivo << "|" << e.what() << "\n";
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cout << "[CRASH] Ocorreu uma excecao inesperada no sistema: " << e.what() << "\n";
        return false;
    }
    return false;
}

int main() {
    int testesTotais = 0;
    int testesPassados = 0;

    std::cout << "==================================================\n";
    std::cout << "  SUITE DE TESTES INTEGRADOS: LEXER+PARSER+SEMANTICO+INTERPRETER \n";
    std::cout << "==================================================\n\n";

    //Varre a pasta de testes que devem compilar com sucesso total
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

    //Varre a pasta de testes que devem falhar na gramatica (Lexer/Parser)
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

    //Varre a pasta de testes que devem falhar nas regras de tipo/escopo (Typechecker)
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

    //Varre a pasta de testes que devem falhar na execução (Interpreter)
    std::string pastaInvalidosRuntime = "../testes/invalidos_runtime";
    if (fs::exists(pastaInvalidosRuntime)) {
        std::cout << "--- Testando Erros de Runtime Esperados ---\n";
        for (const auto& entry : fs::directory_iterator(pastaInvalidosRuntime)) {
            if (entry.path().extension() == ".mc" || entry.path().extension() == ".cpp") {
                testesTotais++;
                if (testarArquivo(entry.path().string(), Expectativa::ErroRuntime)) {
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
