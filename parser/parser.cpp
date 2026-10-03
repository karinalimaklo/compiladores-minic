#include "parser.hpp"

Parser::Parser(std::vector<Token> tokensEntrada) : tokens(std::move(tokensEntrada)) {
    // Garante que o vetor sempre termina em FimArquivo (espiar depende disso)
    if (tokens.empty() || tokens.back().tipo != TipoToken::FimArquivo) {
        Posicao p = tokens.empty() ? Posicao{} : tokens.back().posicao;
        tokens.push_back(Token{TipoToken::FimArquivo, "", p});
    }
}

// programa ::= declaracao_funcao* funcao_main
std::unique_ptr<ProgramAST> Parser::parsePrograma() {
    auto prog = std::make_unique<ProgramAST>();
    while (!checar(TipoToken::FimArquivo)) {
        prog->funcoes.push_back(parseDeclaracaoFuncao());
    }

    // funcao_main ::= "int" "main" "(" ")" bloco   (tem que ser a ultima)
    if (prog->funcoes.empty()) {
        erro("Programa vazio: esperada a funcao 'int main()'");
    }
    const FunctionAST& ultima = *prog->funcoes.back();
    if (ultima.nome != "main" || ultima.tipoRetorno != "int" || !ultima.parametros.empty()) {
        throw ErroSintatico("Erro sintatico: o programa deve terminar com a funcao 'int main()'",
                            atual().posicao);
    }
    return prog;
}

// declaracao_funcao ::= tipo_retorno identificador "(" parametros? ")" bloco
std::unique_ptr<FunctionAST> Parser::parseDeclaracaoFuncao() {
    std::string tipoRet = atual().lexeme;
    if (checar(TipoToken::PalVoid) || ehTipoBasico(atual().tipo)) {
        pos++;
    } else {
        erro("Esperado tipo de retorno da funcao (int, bool, char, double ou void)");
    }

    Token nomeTok = consumir(TipoToken::Ident, "Esperado nome da funcao");
    consumir(TipoToken::ColEsquerda, "Esperado '(' apos o nome da funcao");

    std::vector<ParamAST> params;
    if (ehTipoBasico(atual().tipo)) {
        params = parseParametros();
    }

    consumir(TipoToken::ColDireita, "Esperado ')' apos os parametros");
    auto corpo = parseBloco();

    return std::make_unique<FunctionAST>(tipoRet, nomeTok.lexeme, std::move(params),
                                         std::move(corpo));
}

// parametros ::= parametro ("," parametro)*
std::vector<ParamAST> Parser::parseParametros() {
    std::vector<ParamAST> params;
    params.push_back(parseParametro());
    while (checar(TipoToken::Virg)) {
        pos++;
        params.push_back(parseParametro());
    }
    return params;
}

// parametro ::= tipo_basico identificador
ParamAST Parser::parseParametro() {
    std::string tipo = atual().lexeme;
    if (!ehTipoBasico(atual().tipo)) {
        erro("Esperado tipo do parametro");
    }
    pos++;
    Token nomeTok = consumir(TipoToken::Ident, "Esperado nome do parametro");
    return {tipo, nomeTok.lexeme};
}

// bloco ::= "{" comando* "}"
std::unique_ptr<BlockStmtAST> Parser::parseBloco() {
    consumir(TipoToken::ChavesEsq, "Esperado '{'");
    std::vector<StmtPtr> comandos;

    while (!checar(TipoToken::ChavesDir) && !checar(TipoToken::FimArquivo)) {
        if (ehTipoBasico(atual().tipo)) {
            // Declaracao multipla (ex: int a, b = 2;) vira um VarDecl por variavel
            auto decls = parseDeclaracaoVariavel();
            for (auto& d : decls) comandos.push_back(std::move(d));
        } else {
            comandos.push_back(parseComando());
        }
    }

    consumir(TipoToken::ChavesDir, "Esperado '}'");
    return std::make_unique<BlockStmtAST>(std::move(comandos));
}

// comando ::= declaracao_variavel | atribuicao | comando_if | comando_while
//           | comando_break | comando_continue | comando_return | bloco | expressao ";"
StmtPtr Parser::parseComando() {
    if (checar(TipoToken::PalIf)) return parseComandoIf();
    if (checar(TipoToken::PalWhile)) return parseComandoWhile();

    if (checar(TipoToken::PalBreak)) {
        pos++;
        consumir(TipoToken::PontoVirg, "Esperado ';' apos 'break'");
        return std::make_unique<BreakStmtAST>();
    }
    if (checar(TipoToken::PalContinue)) {
        pos++;
        consumir(TipoToken::PontoVirg, "Esperado ';' apos 'continue'");
        return std::make_unique<ContinueStmtAST>();
    }
    if (checar(TipoToken::PalRet)) return parseComandoReturn();
    if (checar(TipoToken::ChavesEsq)) return parseBloco();

    // Declaracao fora de um bloco direto (ex: if (x) int a = 1;).
    // A gramatica permite; com uma variavel vira o proprio VarDecl,
    // com varias vira um bloco com os VarDecls.
    if (ehTipoBasico(atual().tipo)) {
        auto decls = parseDeclaracaoVariavel();
        if (decls.size() == 1) return std::move(decls[0]);
        return std::make_unique<BlockStmtAST>(std::move(decls));
    }

    if (checar(TipoToken::Ident) && espiar(1).tipo == TipoToken::Assign) {
        return parseAtribuicao();
    }

    auto expr = parseExpressao();
    consumir(TipoToken::PontoVirg, "Esperado ';' no final da expressao");
    return std::make_unique<ExprStmtAST>(std::move(expr));
}

// declaracao_variavel ::= tipo_basico declarador ("," declarador)* ";"
// declarador ::= identificador ("=" expressao)?
std::vector<StmtPtr> Parser::parseDeclaracaoVariavel() {
    std::string tipo = atual().lexeme;
    pos++;  // consome o tipo basico

    std::vector<StmtPtr> decls;

    auto parseDeclarador = [this, &tipo]() -> StmtPtr {
        Token nomeTok = consumir(TipoToken::Ident, "Esperado nome da variavel");
        ExprPtr init = nullptr;
        if (checar(TipoToken::Assign)) {
            pos++;
            init = parseExpressao();
        }
        return std::make_unique<VarDeclStmtAST>(tipo, nomeTok.lexeme, std::move(init));
    };

    decls.push_back(parseDeclarador());
    while (checar(TipoToken::Virg)) {
        pos++;
        decls.push_back(parseDeclarador());
    }
    consumir(TipoToken::PontoVirg, "Esperado ';' no final da declaracao");
    return decls;
}

// atribuicao ::= identificador "=" expressao ";"
StmtPtr Parser::parseAtribuicao() {
    Token nomeTok = consumir(TipoToken::Ident, "Esperado identificador");
    consumir(TipoToken::Assign, "Esperado '='");
    auto expr = parseExpressao();
    consumir(TipoToken::PontoVirg, "Esperado ';' no final da atribuicao");
    return std::make_unique<AssignStmtAST>(nomeTok.lexeme, std::move(expr));
}

// comando_if ::= "if" "(" expressao ")" comando ("else" comando)?
StmtPtr Parser::parseComandoIf() {
    consumir(TipoToken::PalIf, "Esperado 'if'");
    consumir(TipoToken::ColEsquerda, "Esperado '(' apos 'if'");
    auto cond = parseExpressao();
    consumir(TipoToken::ColDireita, "Esperado ')' apos a condicao do if");
    auto entao = parseComando();

    StmtPtr senao = nullptr;
    if (checar(TipoToken::PalElse)) {
        pos++;
        senao = parseComando();
    }
    return std::make_unique<IfStmtAST>(std::move(cond), std::move(entao), std::move(senao));
}

// comando_while ::= "while" "(" expressao ")" comando
StmtPtr Parser::parseComandoWhile() {
    consumir(TipoToken::PalWhile, "Esperado 'while'");
    consumir(TipoToken::ColEsquerda, "Esperado '(' apos 'while'");
    auto cond = parseExpressao();
    consumir(TipoToken::ColDireita, "Esperado ')' apos a condicao do while");
    auto corpo = parseComando();
    return std::make_unique<WhileStmtAST>(std::move(cond), std::move(corpo));
}

// comando_return ::= "return" expressao? ";"
StmtPtr Parser::parseComandoReturn() {
    consumir(TipoToken::PalRet, "Esperado 'return'");
    ExprPtr expr = nullptr;
    if (!checar(TipoToken::PontoVirg)) {
        expr = parseExpressao();
    }
    consumir(TipoToken::PontoVirg, "Esperado ';' apos 'return'");
    return std::make_unique<ReturnStmtAST>(std::move(expr));
}

// ==========================================
// EXPRESSOES (uma funcao por nivel de precedencia,
// operadores binarios associativos a esquerda)
// ==========================================

// expressao ::= expressao_or
ExprPtr Parser::parseExpressao() {
    return parseExpressaoOr();
}

// expressao_or ::= expressao_and ("||" expressao_and)*
ExprPtr Parser::parseExpressaoOr() {
    auto esq = parseExpressaoAnd();
    while (checar(TipoToken::OrOr)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseExpressaoAnd();
        esq = std::make_unique<BinaryExprAST>(op, std::move(esq), std::move(dir));
    }
    return esq;
}

// expressao_and ::= expressao_igualdade ("&&" expressao_igualdade)*
ExprPtr Parser::parseExpressaoAnd() {
    auto esq = parseExpressaoIgualdade();
    while (checar(TipoToken::AndAnd)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseExpressaoIgualdade();
        esq = std::make_unique<BinaryExprAST>(op, std::move(esq), std::move(dir));
    }
    return esq;
}

// expressao_igualdade ::= expressao_relacional (op_igualdade expressao_relacional)*
ExprPtr Parser::parseExpressaoIgualdade() {
    auto esq = parseExpressaoRelacional();
    while (checar(TipoToken::Eq) || checar(TipoToken::NotEq)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseExpressaoRelacional();
        esq = std::make_unique<BinaryExprAST>(op, std::move(esq), std::move(dir));
    }
    return esq;
}

// expressao_relacional ::= expr_aritmetica (op_relacional expr_aritmetica)*
ExprPtr Parser::parseExpressaoRelacional() {
    auto esq = parseExprAritmetica();
    while (checar(TipoToken::Lt) || checar(TipoToken::Le) ||
           checar(TipoToken::Gt) || checar(TipoToken::Ge)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseExprAritmetica();
        esq = std::make_unique<BinaryExprAST>(op, std::move(esq), std::move(dir));
    }
    return esq;
}

// expr_aritmetica ::= termo (op_aditivo termo)*
ExprPtr Parser::parseExprAritmetica() {
    auto esq = parseTermo();
    while (checar(TipoToken::Plus) || checar(TipoToken::Min)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseTermo();
        esq = std::make_unique<BinaryExprAST>(op, std::move(esq), std::move(dir));
    }
    return esq;
}

// termo ::= fator (op_multiplicativo fator)*
ExprPtr Parser::parseTermo() {
    auto esq = parseFator();
    while (checar(TipoToken::Star) || checar(TipoToken::Div) || checar(TipoToken::Percent)) {
        std::string op = atual().lexeme;
        pos++;
        auto dir = parseFator();
        esq = std::make_unique<BinaryExprAST>(op, std::move(esq), std::move(dir));
    }
    return esq;
}

// fator ::= literal | chamada_funcao | incremento | identificador
//         | "(" expressao ")" | op_unario fator
ExprPtr Parser::parseFator() {
    if (checar(TipoToken::Not)) {
        std::string op = atual().lexeme;
        pos++;
        return std::make_unique<UnaryExprAST>(op, parseFator());
    }

    if (checar(TipoToken::ColEsquerda)) {
        pos++;
        auto expr = parseExpressao();
        consumir(TipoToken::ColDireita, "Esperado ')' para fechar a expressao");
        return expr;
    }

    if (checar(TipoToken::IntLit) || checar(TipoToken::DoubleLit) ||
        checar(TipoToken::CharLit) || checar(TipoToken::PalTrue) ||
        checar(TipoToken::PalFalse)) {
        std::string val = atual().lexeme;
        pos++;
        return std::make_unique<LiteralExprAST>(val);
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
            return std::make_unique<IncrementExprAST>(var, op);
        }
        std::string var = atual().lexeme;
        pos++;
        return std::make_unique<VariableExprAST>(var);
    }

    erro("Esperada uma expressao (literal, variavel, chamada, '(' ou '!')");
}

// chamada_funcao ::= identificador "(" argumentos? ")"
ExprPtr Parser::parseChamadaFuncao() {
    Token nomeTok = consumir(TipoToken::Ident, "Esperado nome da funcao");
    consumir(TipoToken::ColEsquerda, "Esperado '(' na chamada de funcao");

    std::vector<ExprPtr> args;
    if (!checar(TipoToken::ColDireita)) {
        args = parseArgumentos();
    }

    consumir(TipoToken::ColDireita, "Esperado ')' no final da chamada de funcao");
    return std::make_unique<CallExprAST>(nomeTok.lexeme, std::move(args));
}

// argumentos ::= expressao ("," expressao)*
std::vector<ExprPtr> Parser::parseArgumentos() {
    std::vector<ExprPtr> args;
    args.push_back(parseExpressao());
    while (checar(TipoToken::Virg)) {
        pos++;
        args.push_back(parseExpressao());
    }
    return args;
}
