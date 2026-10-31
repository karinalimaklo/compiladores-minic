// Representação de valores e exceções

#pragma once

#include <variant>
#include <string>
#include <memory>
#include <unordered_map>
#include <stdexcept>

// Exceção disparada em caso de erros de erros de execução (ex.: divisão por 0)
class RuntimeException : public std::runtime_error {
    public:
        explicit RuntimeException(const std::string& msg) : std::runtime_error(msg) {}
};

struct ObjectValue;

// Tipo polimórfico de tempo de execução que suporta primitivos e ponteiros/instâncias de objetos
using Value = std::variant<int, double, char, bool, std::shared_ptr<ObjectValue>>;

// Estrutura que representa uma instância de struct ou classe na memória dinamicamente alocada
struct ObjectValue {
    std::string nomeClasse;
    std::unordered_map<std::string, Value> campos; //mapeia nome do atributo -> valor atual
};