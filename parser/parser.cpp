#include "parser.hpp"

std::unique_ptr Parser::parsePrograma() {
    auto prog = std::make_unique();
    while (!checar(TipoToken::FimArquivo)) {
        prog->funcoes.push_back(parseDeclaracaoFuncao());
    }
    return prog;
}

std::unique_ptr Parser::parseDeclaracaoFuncao() {
    std::string tipoRet = atual().lexeme;
    if (checar(TipoToken::PalVoid) || ehTipoBasico(atual().tipo)) {
        pos++;
    } else {
        throw std::runtime_error("Esperado tipo de retorno de funcao.");
    }

    Token nomeTok = consumir(TipoToken::Ident, "Esperado nome da funcao.");
    consumir(TipoToken::ColEsquerda, "Esperado '(' apos nome da funcao.");

    std::vector params;
    if (ehTipoBasico(atual().tipo)) {
        params = parseParametros();
    }

    consumir(TipoToken::ColDireita, "Esperado ')' apos parametros.");
    auto corpo = parseBloco();

    return std::make_unique(tipoRet, nomeTok.lexeme, std::move(params), std::move(corpo));
}

std::vector Parser::parseParametros() {
    std::vector params;
    params.push_back(parseParametro());
    while (checar(TipoToken::Virg)) {
        consumir(TipoToken::Virg, "");
        params.push_back(parseParametro());
    }
    return params;
}

ParamAST Parser::parseParametro() {
    std::string tipo = atual().lexeme;
    if (!ehTipoBasico(atual().tipo)) {
        throw std::runtime_error("Esperado tipo do parametro.");
    }
    pos++;
    Token nomeTok = consumir(TipoToken::Ident, "Esperado nome do parametro.");
    return {tipo, nomeTok.lexeme};
}

std::unique_ptr Parser::parseBloco() {
    consumir(TipoToken::ChavesEsq, "Esperado '{'.");
    std::vector> comandos;

    while (!checar(TipoToken::ChavesDir) && !checar(TipoToken::FimArquivo)) {
        if (ehTipoBasico(atual().tipo)) {
            // Trata declaração múltipla (ex: int a, b = 2;) inserindo cada uma no bloco
            auto decls = parseDeclaracaoVariavel();
            for (auto& d : decls) comandos.push_back(std::move(d));
        } else {
            comandos.push_back(parseComando());
        }
    }

    consumir(TipoToken::ChavesDir, "Esperado '}'.");
    return std::make_unique(std::move(comandos));
}

std::unique_ptr Parser::parseComando() {
    if (checar(TipoToken::PalIf)) return parseComandoIf();
    if (checar(TipoToken::PalWhile)) return parseComandoWhile();
    
    if (checar(TipoToken::PalBreak)) {
        pos++;
        consumir(TipoToken::PontoVirg, "Esperado ';' apos 'break'.");
        return std::make_unique(std::make_unique("break"));
    }
    if (checar(TipoToken::PalContinue)) {
        pos++;
        consumir(TipoToken::PontoVirg, "Esperado ';' apos 'continue'.");
        return std::make_unique(std::make_unique("continue"));
    }
    if (checar(TipoToken::PalRet)) return parseComandoReturn();
    if (checar(TipoToken::ChavesEsq)) return parseBloco();

    if (checar(TipoToken::Ident) && espiar(1).tipo == TipoToken::Assign) {
        return parseAtribuicao();
    }

    auto expr = parseExpressao();
    consumir(TipoToken::PontoVirg, "Esperado ';' no final da expressao.");
    return std::make_unique(std::move(expr));
}

std::vector> Parser::parseDeclaracaoVariavel() {
    std::string tipo = atual().lexeme;
    pos++; // consome o tipo básico
    
    std::vector> decls;

    auto parseDeclarador = [this, &tipo]() -> std::unique_ptr {
        Token nomeTok = consumir(TipoToken::Ident, "Esperado nome da variavel.");
        std::unique_ptr init = nullptr;
        if (checar(TipoToken::Assign)) {
            consumir(TipoToken::Assign, "");
            init = parseExpressao();
        }
        return std::make_unique(tipo, nomeTok.lexeme, std::move(init));
    };

    decls.push_back(parseDeclarador());
    while (checar(TipoToken::Virg)) {
        consumir(TipoToken::Virg, "");
        decls.push_back(parseDeclarador());
    }
    consumir(TipoToken::PontoVirg, "Esperado ';'.");
    return decls;
}

std::unique_ptr Parser::parseAtribuicao() {
    Token nomeTok = consumir(TipoToken::Ident, "Esperado identificador.");
    consumir(TipoToken::Assign, "");
    auto expr = parseExpressao();
    consumir(TipoToken::PontoVirg, "Esperado ';'.");
    return std::make_unique(nomeTok.lexeme, std::move(expr));
}

std::unique_ptr Parser::parseComandoIf() {
    consumir(TipoToken::PalIf, "");
    consumir(TipoToken::ColEsquerda, "");
    auto cond = parseExpressao();
    consumir(TipoToken::ColDireita, "");
    auto então = parseComando();

    std::unique_ptr senão = nullptr;
    if (checar(TipoToken::PalElse)) {
        consumir(TipoToken::PalElse, "");
        senão = parseComando();
    }
    return std::make_unique(std::move(cond), std::move(então), std::move(senão));
}

std::unique_ptr Parser::parseComandoWhile() {
    consumir(TipoToken::PalWhile, "");
    consumir(TipoToken::ColEsquerda, "");
    auto cond = parseExpressao();
    consumir(TipoToken::ColDireita, "");
    auto corpo = parseComando();
    return std::make_unique(std::move(cond), std::move(corpo));
}

std::unique_ptr Parser::parseComandoReturn() {
    consumir(TipoToken::PalRet, "");
    std::unique_ptr expr = nullptr;
    if (!checar(TipoToken::PontoVirg)) {
        expr = parseExpressao();
    }
    consumir(TipoToken::PontoVirg, "");
    return std::make_unique(std::move(expr));
}

// ==========================================
// MÉTODOS DE EXPRESSÃO (CRIAÇÃO DE NÓS BINÁRIOS/UNÁRIOS)
// ==========================================
std::unique_ptr Parser::parseExpressao() {
    return parseExpressaoOr();
}

std::unique_ptr Parser::parseExpressaoOr() {
    auto esq = parseExpressaoAnd();
    while (checar(TipoToken::OrOr)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseExpressaoAnd();
        esq = std::make_unique(op, std::move(esq), std::move(dir));
    }
    return esq;
}

std::unique_ptr Parser::parseExpressaoAnd() {
    auto esq = parseExpressaoIgualdade();
    while (checar(TipoToken::AndAnd)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseExpressaoIgualdade();
        esq = std::make_unique(op, std::move(esq), std::move(dir));
    }
    return esq;
}

std::unique_ptr Parser::parseExpressaoIgualdade() {
    auto esq = parseExpressaoRelacional();
    while (checar(TipoToken::Eq) || checar(TipoToken::NotEq)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseExpressaoRelacional();
        esq = std::make_unique(op, std::move(esq), std::move(dir));
    }
    return esq;
}

std::unique_ptr Parser::parseExpressaoRelacional() {
    auto esq = parseExprAritmetica();
    while (checar(TipoToken::Lt) || checar(TipoToken::Le) || 
           checar(TipoToken::Gt) || checar(TipoToken::Ge)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseExprAritmetica();
        esq = std::make_unique(op, std::move(esq), std::move(dir));
    }
    return esq;
}

std::unique_ptr Parser::parseExprAritmetica() {
    auto esq = parseTermo();
    while (checar(TipoToken::Plus) || checar(TipoToken::Min)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseTermo();
        esq = std::make_unique(op, std::move(esq), std::move(dir));
    }
    return esq;
}

std::unique_ptr Parser::parseTermo() {
    auto esq = parseFator();
    while (checar(TipoToken::Star) || checar(TipoToken::Div) || checar(TipoToken::Percent)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseFator();
        esq = std::make_unique(op, std::move(esq), std::move(dir));
    }
    return esq;
}

std::unique_ptr Parser::parseFator() {
    if (checar(TipoToken::Not)) {
        std::string op = atual().lexeme;
        pos++;
        return std::make_unique(op, parseFator());
    }

    if (checar(TipoToken::ColEsquerda)) {
        consumir(TipoToken::ColEsquerda, "");
        auto expr = parseExpressao();
        consumir(TipoToken::ColDireita, "Esperado ')' fechar a expressao.");
        return expr;
    }

    if (checar(TipoToken::IntLit) || checar(TipoToken::DoubleLit) || 
        checar(TipoToken::CharLit) || checar(TipoToken::PalTrue) || 
        checar(TipoToken::PalFalse)) {
        std::string val = atual().lexeme;
        pos++;
        return std::make_unique(val);
    }

    if (checar(TipoToken::Ident)) {
        if (espiar(1).tipo == TipoToken::ColEsquerda) {
            return parseChamadaFuncao();
        } 
        if (espiar(1).tipo == TipoToken::PlusPLus || espiar(1).tipo == TipoToken::MinMin) {
            std::string var = atual().lexeme;
            pos++;
            std::string op = atual().lexeme;
            pos++;
            return std::make_unique(var, op);
        } 
        std::string var = atual().lexeme;
        pos++;
        return std::make_unique(var);
    }

    throw std::runtime_error("Fator invalido na expressao.");
}

std::unique_ptr Parser::parseChamadaFuncao() {
    Token nomeTok = consumir(TipoToken::Ident, "");
    consumir(TipoToken::ColEsquerda, "");
    
    std::vector> args;
    if (!checar(TipoToken::ColDireita)) {
        args = parseArgumentos();
    }
    
    consumir(TipoToken::ColDireita, "");
    return std::make_unique(nomeTok.lexeme, std::move(args));
}

std::vector> Parser::parseArgumentos() {
    std::vector> args;
    args.push_back(parseExpressao());
    while (checar(TipoToken::Virg)) {
        consumir(TipoToken::Virg, "");
        args.push_back(parseExpressao());
    }
    return args;
}