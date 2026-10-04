cmake_minimum_required(VERSION 3.15)
project(MiniCompiler CXX)

# Forçar o uso do C++20 ou superior
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# RESOLVE O AVISO DE RELOCATION (Código Independente de Posição)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 1. Incluir TODOS os arquivos .cpp da pasta lexer que aparecem na imagem
set(LEXER_SOURCES
    lexer/af.cpp
    lexer/afd.cpp
    lexer/afdConstructor.cpp
    lexer/afn.cpp
    lexer/afnConstructor.cpp
    lexer/gramaticaCod.cpp
    lexer/lexer.cpp
    lexer/parserRegex.cpp
    lexer/util.cpp
)

# 2. Mapear os arquivos do Parser
set(PARSER_SOURCES
    parser/parser.cpp
)

# 3. Criar a biblioteca estática com o compilador completo
add_library(compiler_core STATIC 
    ${LEXER_SOURCES} 
    ${PARSER_SOURCES}
)

# Indicar onde procurar as pastas de cabeçalhos (.hpp)
target_include_directories(compiler_core PUBLIC 
    lexer 
    parser
)

# 4. Criar o executável da sua Suite de Testes
add_executable(runner_testes 
    testes/runner_testes_files.cpp
)

# Linkar o executável com o compilador
target_link_libraries(runner_testes PRIVATE compiler_core)
