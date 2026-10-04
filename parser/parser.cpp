#include <vector>
#include "parser.hpp"
#include "../lexer/token.hpp"
// =============================================================================
// CONSTRUTOR E MÉTODOS AUXILIARES DE CONTROLE
// =============================================================================

// O construtor recebe o vetor de tokens via Semântica de Movimento (move)
Parser::Parser(std::vector<Token> tokens) 
    : m_tokens(std::move(tokens)), m_atual(0) {}

// Retorna o token atual sem avançar o ponteiro (Lookahead)
Token Parser::espiar() const {
    return m_tokens[m_atual];
}

// Retorna o token que acabou de ser processado
Token Parser::anterior() const {
    return m_tokens[m_atual - 1];
}

// Verifica se o token atual é o marcador de Fim de Arquivo (EOF)
bool Parser::fim() const {
    return espiar().tipo == TipoToken::FimArquivo;
}

// Verifica se o token atual corresponde ao tipo esperado (sem consumi-lo)
bool Parser::checar(TipoToken tipo) const {
    if (fim()) return false;
    return espiar().tipo == tipo;
}

// Avança o ponteiro do Parser e retorna o token que foi ultrapassado
Token Parser::avancar() {
    if (!fim()) m_atual++;
    return anterior();
}

// =============================================================================
// MÉTODOS DE VALIDAÇÃO E EXTRAÇÃO DE TIPOS
// =============================================================================

// Se o token atual for do tipo esperado, consome-o. Caso contrário, lança erro.
Token Parser::consumir(TipoToken tipo, const std::string& mensagemErro) {
    if (checar(tipo)) return avancar();
    throw ParserException(mensagemErro, espiar().posicao);
}

// Verifica se o token atual bate com algum dos tipos listados. Se sim, consome e retorna true.
bool Parser::match(const std::vector<TipoToken>& tipos) {
    for (TipoToken tipo : tipos) {
        if (checar(tipo)) {
            avancar();
            return true;
        }
    }
    return false;
}

// Converte os tokens de palavras-chave para o enum de tipos abstratos da AST
DataType Parser::mapearTipoBasico(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::PalInt:    return DataType::Int;
        case TipoToken::PalBool:   return DataType::Bool;
        case TipoToken::PalChar:   return DataType::Char;
        case TipoToken::PalDouble: return DataType::Double;
        case TipoToken::PalVoid:   return DataType::Void;
        default:
            throw ParserException("Erro Sintatico: Tipo de dado invalido ou nao suportado.", espiar().posicao);
    }
}

// =============================================================================
// REGRAS DE PARÂMETROS E ARGUMENTOS DE FUNÇÕES
// =============================================================================

// Processa a lista de parâmetros: parametro ("," parametro)*
std::vector<Parameter> Parser::parseParametros() {
    std::vector<Parameter> lista;
    lista.push_back(parseParametro());
    
    while (match({TipoToken::Virg})) {
        lista.push_back(parseParametro());
    }
    return lista;
}

// Processa um único parâmetro: tipo_basico identificador
Parameter Parser::parseParametro() {
    if (!checar(TipoToken::PalInt) && !checar(TipoToken::PalChar) && 
        !checar(TipoToken::PalDouble) && !checar(TipoToken::PalBool)) {
        throw ParserException("Erro Sintatico: Esperado tipo de dado basico para o parametro.", espiar().posicao);
    }
    
    DataType tipo = mapearTipoBasico(avancar().tipo);
    Token idToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado identificador para o parametro.");
    
    return Parameter{tipo, idToken.lexeme};
}

// Processa os argumentos passados em chamadas: expressao ("," expressao)*
std::vector<std::unique_ptr<ExpressionNode>> Parser::parseArgumentos() {
    std::vector<std::unique_ptr<ExpressionNode>> lista;
    lista.push_back(parseExpressao());
    
    while (match({TipoToken::Virg})) {
        lista.push_back(parseExpressao());
    }
    return lista;
}

// =============================================================================
// ESTRUTURA GLOBAL DO PROGRAMA E DEFINIÇÃO DE FUNÇÕES
// =============================================================================

// Ponto de entrada principal: programa ::= declaracao_funcao* funcao_main
std::unique_ptr<ProgramNode> Parser::parse() {
    std::vector<std::unique_ptr<FunctionNode>> funcoes;
    std::unique_ptr<FunctionNode> funcaoMain = nullptr;

    while (!fim()) {
        // Lookahead: se o tipo for 'int' e o próximo lexema for 'main', achamos a main
        if (checar(TipoToken::PalInt) && 
            (m_atual + 1 < m_tokens.size() && m_tokens[m_atual + 1].lexeme == "main")) {
            funcaoMain = parseFuncaoMain();
            break; // Segundo a gramática, a main é o último elemento do arquivo
        } else {
            funcoes.push_back(parseDeclaracaoFuncao());
        }
    }

    if (!funcaoMain) {
        throw ParserException("Erro Sintatico: Funcao obrigatoria 'int main()' nao encontrada ou fora de posicao.", espiar().posicao);
    }

    return std::make_unique<ProgramNode>(std::move(funcoes), std::move(funcaoMain));
}

// Processa uma função comum: tipo_retorno identificador "(" parametros? ")" bloco
std::unique_ptr<FunctionNode> Parser::parseDeclaracaoFuncao() {
    if (!checar(TipoToken::PalInt) && !checar(TipoToken::PalChar) && 
        !checar(TipoToken::PalDouble) && !checar(TipoToken::PalBool) && !checar(TipoToken::PalVoid)) {
        throw ParserException("Erro Sintatico: Esperado tipo de retorno valido para a funcao.", espiar().posicao);
    }
    
    DataType tipoRet = mapearTipoBasico(avancar().tipo);
    Token idToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado identificador com o nome da funcao.");
    
    consumir(TipoToken::ColEsquerda, "Erro Sintatico: Esperado '(' apos o nome da funcao.");
    std::vector<Parameter> params;
    if (!checar(TipoToken::ColDireita)) {
        params = parseParametros();
    }
    consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos a lista de parametros.");

    // Chama o analisador de bloco de comandos: { comando* }
    std::unique_ptr<BlockStmtNode> corpo = parseBloco();

    return std::make_unique<FunctionNode>(tipoRet, idToken.lexeme, std::move(params), std::move(corpo));
}

// Processa especificamente a main: "int" "main" "(" ")" bloco
std::unique_ptr<FunctionNode> Parser::parseFuncaoMain() {
    consumir(TipoToken::PalInt, "Erro Sintatico: O tipo de retorno da funcao main deve ser obrigatoriamente 'int'.");
    
    Token mainId = consumir(TipoToken::Ident, "Erro Sintatico: Esperado identificador 'main'.");
    if (mainId.lexeme != "main") {
        throw ParserException("Erro Sintatico: Identificador inválido para a funcao principal, esperado 'main'.", mainId.posicao);
    }

    consumir(TipoToken::ColEsquerda, "Erro Sintatico: Esperado '(' na assinatura da funcao main.");
    consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' na assinatura da funcao main.");

    // Processa o corpo da main
    std::unique_ptr<BlockStmtNode> corpo = parseBloco();

    return std::make_unique<FunctionNode>(DataType::Int, "main", std::vector<Parameter>{}, std::move(corpo));
}

// =============================================================================
// REGRAS DE BLOCO E COMANDOS DE CONTROLE DE FLUXO
// =============================================================================

// Processa um bloco fechado: bloco ::= "{" comando* "}"
std::unique_ptr<BlockStmtNode> Parser::parseBloco() {
    consumir(TipoToken::ChavesEsq, "Erro Sintatico: Esperado '{' para iniciar o bloco de comandos.");
    std::vector<std::unique_ptr<StatementNode>> comandos;
    
    // Continua consumindo comandos até encontrar o fechamento ou o fim do arquivo
    while (!checar(TipoToken::ChavesDir) && !fim()) {
        comandos.push_back(parseComando());
    }
    
    consumir(TipoToken::ChavesDir, "Erro Sintatico: Esperado '}' para fechar o bloco de comandos.");
    return std::make_unique<BlockStmtNode>(std::move(comandos));
}

// Processa condicionais: comando_if ::= "if" "(" expressao ")" comando ("else" comando)?
std::unique_ptr<StatementNode> Parser::parseComandoIf() {
    avancar(); // Consome o token 'if'
    consumir(TipoToken::ColEsquerda, "Erro Sintatico: Esperado '(' apos a palavra-chave 'if'.");
    std::unique_ptr<ExpressionNode> condicao = parseExpressao();
    consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos a condicao do 'if'.");
    
    std::unique_ptr<StatementNode> comandoThen = parseComando();
    std::unique_ptr<StatementNode> comandoElse = nullptr;

    // Verifica a presença opcional do 'else'
    if (match({TipoToken::PalElse})) {
        comandoElse = parseComando();
    }

    return std::make_unique<IfStmtNode>(std::move(condicao), std::move(comandoThen), std::move(comandoElse));
}

// Processa laços: comando_while ::= "while" "(" expressao ")" comando
std::unique_ptr<StatementNode> Parser::parseComandoWhile() {
    avancar(); // Consome o token 'while'
    consumir(TipoToken::ColEsquerda, "Erro Sintatico: Esperado '(' apos a palavra-chave 'while'.");
    std::unique_ptr<ExpressionNode> condicao = parseExpressao();
    consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos a condicao do 'while'.");
    
    std::unique_ptr<StatementNode> comandoBody = parseComando();
    return std::make_unique<WhileStmtNode>(std::move(condicao), std::move(comandoBody));
}

// Processa quebras de laço: comando_break ::= "break" ";"
std::unique_ptr<StatementNode> Parser::parseComandoBreak() {
    avancar(); // Consome o token 'break'
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos o comando 'break'.");
    return std::make_unique<BreakStmtNode>();
}

// Processa continuação de laço: comando_continue ::= "continue" ";"
std::unique_ptr<StatementNode> Parser::parseComandoContinue() {
    avancar(); // Consome o token 'continue'
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos o comando 'continue'.");
    return std::make_unique<ContinueStmtNode>();
}

// =============================================================================
// DESPACHANTE DE COMANDOS E RETORNO
// =============================================================================

// Processa o retorno: comando_return ::= "return" expressao? ";"
std::unique_ptr<StatementNode> Parser::parseComandoReturn() {
    avancar(); // Consome o token 'return'
    std::unique_ptr<ExpressionNode> expressao = nullptr;
    
    // Se o próximo token não for um ponto e vírgula, significa que há uma expressão de retorno
    if (!checar(TipoToken::PontoVirg)) {
        expressao = parseExpressao();
    }
    
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos o comando 'return'.");
    return std::make_unique<ReturnStmtNode>(std::move(expressao));
}

// Roteador central: comando ::= declaracao_variavel | atribuicao | comando_if | ...
std::unique_ptr<StatementNode> Parser::parseComando() {
    // Se começar com uma palavra-chave de tipo básico, é uma declaração de variável
    if (checar(TipoToken::PalInt) || checar(TipoToken::PalBool) || 
        checar(TipoToken::PalChar) || checar(TipoToken::PalDouble)) {
        return parseDeclaracaoVariavel();
    }
    
    // Encaminha para os comandos de controle correspondentes
    if (checar(TipoToken::PalIf))       return parseComandoIf();
    if (checar(TipoToken::PalWhile))    return parseComandoWhile();
    if (checar(TipoToken::PalBreak))    return parseComandoBreak();
    if (checar(TipoToken::PalContinue)) return parseComandoContinue();
    if (checar(TipoToken::PalRet))      return parseComandoReturn();
    if (checar(TipoToken::ChavesEsq))   return parseBloco();

    // Se não cair em nenhuma das opções acima, restam duas possibilidades da gramática:
    // Uma atribuição (id = expr;) ou uma expressão pura terminada em ';' (ex: id++; ou chamadas de função)
    return parseAtribuicaoOuExpressaoStmt();
}

// =============================================================================
// DECLARAÇÃO DE VARIÁVEIS E RESOLUÇÃO DE ATRIBUIÇÕES
// =============================================================================

// Processa variáveis: tipo_basico declarador ("," declarador)* ";"
std::unique_ptr<StatementNode> Parser::parseDeclaracaoVariavel() {
    // Coleta o tipo básico (int, bool, char, double) e avança
    DataType tipo = mapearTipoBasico(avancar().tipo);
    std::vector<VariableDeclarator> dees;

    // Processa pelo menos um declarador, repetindo enquanto houver vírgula
    do {
        Token idToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado nome da variavel.");
        std::unique_ptr<ExpressionNode> inicializador = nullptr;
        
        // Verifica se há uma inicialização opcional (ex: = 10)
        if (match({TipoToken::Assign})) {
            inicializador = parseExpressao();
        }
        
        dees.push_back(VariableDeclarator{idToken.lexeme, std::move(inicializador)});
    } while (match({TipoToken::Virg}));

    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos a declaracao de variavel(is).");
    return std::make_unique<VariableDeclStmtNode>(tipo, std::move(dees));
}

// Resolve a ambiguidade lookahead: atribuição (id = expr;) vs expressão isolada (id++; ou foo();)
std::unique_ptr<StatementNode> Parser::parseAtribuicaoOuExpressaoStmt() {
    // Lookahead inteligente: se o token atual for um identificador E o próximo for um '='
    if (checar(TipoToken::Ident) && 
        (m_atual + 1 < m_tokens.size() && m_tokens[m_atual + 1].tipo == TipoToken::Assign)) {
        
        Token idToken = avancar(); // Consome o identificador
        avancar();                 // Consome o '='
        
        std::unique_ptr<ExpressionNode> expr = parseExpressao();
        consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' ao final da atribuicao.");
        
        return std::make_unique<AssignmentStmtNode>(idToken.lexeme, std::move(expr));
    }

    // Se não for uma atribuição direta com '=', cai obrigatoriamente na regra de expressão pura (ex: x++; ou chamar_funcao();)
    std::unique_ptr<ExpressionNode> expr = parseExpressao();
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' ao final da expressao.");
    
    return std::make_unique<ExpressionStmtNode>(std::move(expr));
}

// =============================================================================
// EXPRESSÕES: OPERADORES LÓGICOS E IGUALDADE (BAIXA PRECEDÊNCIA)
// =============================================================================

// Ponto de entrada das expressões: expressao ::= expressao_or
std::unique_ptr<ExpressionNode> Parser::parseExpressao() {
    return parseExpressaoOr();
}

// Processa o operador OU lógico: expressao_and ( "||" expressao_and )*
std::unique_ptr<ExpressionNode> Parser::parseExpressaoOr() {
    std::unique_ptr<ExpressionNode> expr = parseExpressaoAnd();
    
    // Transforma a recursão à esquerda da gramática em um laço iterativo estável
    while (match({TipoToken::OrOr})) {
        std::unique_ptr<ExpressionNode> direita = parseExpressaoAnd();
        expr = std::make_unique<BinaryExprNode>(BinaryOp::LogicalOr, std::move(expr), std::move(direita));
    }
    return expr;
}

// Processa o operador E lógico: expressao_igualdade ( "&&" expressao_igualdade )*
std::unique_ptr<ExpressionNode> Parser::parseExpressaoAnd() {
    std::unique_ptr<ExpressionNode> expr = parseExpressaoIgualdade();
    
    while (match({TipoToken::AndAnd})) {
        std::unique_ptr<ExpressionNode> direita = parseExpressaoIgualdade();
        expr = std::make_unique<BinaryExprNode>(BinaryOp::LogicalAnd, std::move(expr), std::move(direita));
    }
    return expr;
}

// Processa igualdades: expressao_relacional ( ("==" | "!=") expressao_relacional )*
std::unique_ptr<ExpressionNode> Parser::parseExpressaoIgualdade() {
    std::unique_ptr<ExpressionNode> expr = parseExpressaoRelacional();
    
    while (checar(TipoToken::Eq) || checar(TipoToken::NotEq)) {
        Token opToken = avancar(); // Consome o operador
        BinaryOp op = (opToken.tipo == TipoToken::Eq) ? BinaryOp::Equal : BinaryOp::NotEqual;
        
        std::unique_ptr<ExpressionNode> direita = parseExpressaoRelacional();
        expr = std::make_unique<BinaryExprNode>(op, std::move(expr), std::move(direita));
    }
    return expr;
}

// =============================================================================
// EXPRESSÕES: OPERADORES RELACIONAIS E ARITMÉTICOS (MÉDIA PRECEDÊNCIA)
// =============================================================================

// Processa relacionais: expr_aritmetica ( ("<" | "<=" | ">" | ">=") expr_aritmetica )*
std::unique_ptr<ExpressionNode> Parser::parseExpressaoRelacional() {
    std::unique_ptr<ExpressionNode> expr = parseExprAritmetica();
    
    while (checar(TipoToken::Lt) || checar(TipoToken::Le) || checar(TipoToken::Gt) || checar(TipoToken::Ge)) {
        Token opToken = avancar(); // Consome o operador relacional
        BinaryOp op;
        
        if (opToken.tipo == TipoToken::Lt)      op = BinaryOp::LessThan;
        else if (opToken.tipo == TipoToken::Le) op = BinaryOp::LessEqual;
        else if (opToken.tipo == TipoToken::Gt) op = BinaryOp::GreaterThan;
        else                                    op = BinaryOp::GreaterEqual;

        std::unique_ptr<ExpressionNode> direita = parseExprAritmetica();
        expr = std::make_unique<BinaryExprNode>(op, std::move(expr), std::move(direita));
    }
    return expr;
}

// Processa somas e subtrações: termo ( ("+" | "-") termo )*
std::unique_ptr<ExpressionNode> Parser::parseExprAritmetica() {
    std::unique_ptr<ExpressionNode> expr = parseTermo();
    
    while (checar(TipoToken::Plus) || checar(TipoToken::Min)) {
        Token opToken = avancar(); // Consome '+' ou '-'
        BinaryOp op = (opToken.tipo == TipoToken::Plus) ? BinaryOp::Add : BinaryOp::Sub;
        
        std::unique_ptr<ExpressionNode> direita = parseTermo();
        expr = std::make_unique<BinaryExprNode>(op, std::move(expr), std::move(direita));
    }
    return expr;
}

// Processa multiplicações, divisões e restos: fator ( ("*" | "/" | "%") fator )*
std::unique_ptr<ExpressionNode> Parser::parseTermo() {
    std::unique_ptr<ExpressionNode> expr = parseFator();
    
    while (checar(TipoToken::Star) || checar(TipoToken::Div) || checar(TipoToken::Percent)) {
        Token opToken = avancar(); // Consome '*', '/' ou '%'
        BinaryOp op;
        
        if (opToken.tipo == TipoToken::Star)      op = BinaryOp::Mul;
        else if (opToken.tipo == TipoToken::Div) op = BinaryOp::Div;
        else                                     op = BinaryOp::Mod;

        std::unique_ptr<ExpressionNode> direita = parseFator();
        expr = std::make_unique<BinaryExprNode>(op, std::move(expr), std::move(direita));
    }
    return expr;
}

// =============================================================================
// EXPRESSÕES: FATORES (MÁXIMA PRECEDÊNCIA E ELEMENTOS ATÔMICOS)
// =============================================================================

std::unique_ptr<ExpressionNode> Parser::parseFator() {
    // 1. Literais Atômicos
    if (match({TipoToken::IntLit}))    return std::make_unique<LiteralNode>(DataType::Int,    anterior().lexeme);
    if (match({TipoToken::DoubleLit})) return std::make_unique<LiteralNode>(DataType::Double, anterior().lexeme);
    if (match({TipoToken::CharLit}))   return std::make_unique<LiteralNode>(DataType::Char,   anterior().lexeme);
    if (match({TipoToken::PalTrue}))   return std::make_unique<LiteralNode>(DataType::Bool,   "true");
    if (match({TipoToken::PalFalse}))  return std::make_unique<LiteralNode>(DataType::Bool,   "false");

    // 2. Operadores Unários (Prefixo)
    if (match({TipoToken::Not})) {
        std::unique_ptr<ExpressionNode> fat = parseFator();
        return std::make_unique<UnaryExprNode>(UnaryOp::LogicalNot, std::move(fat));
    }
    if (match({TipoToken::Min})) { // Suporte para números negativos (ex: -5)
        std::unique_ptr<ExpressionNode> fat = parseFator();
        return std::make_unique<UnaryExprNode>(UnaryOp::Negate, std::move(fat));
    }

    // 3. Expressões Aninhadas entre Parênteses: "(" expressao ")"
    if (match({TipoToken::ColEsquerda})) {
        std::unique_ptr<ExpressionNode> expr = parseExpressao();
        consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos a expressao aninhada.");
        return expr;
    }

    // 4. Casos que Iniciam com Identificador (Variavel, Chamada ou Incremento)
    if (match({TipoToken::Ident})) {
        Token idToken = anterior();

        // Subregra: chamada_funcao ::= identificador "(" argumentos? ")"
        if (match({TipoToken::ColEsquerda})) {
            std::vector<std::unique_ptr<ExpressionNode>> args;
            if (!checar(TipoToken::ColDireita)) {
                args = parseArgumentos();
            }
            consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos os argumentos da funcao.");
            return std::make_unique<FunctionCallExprNode>(idToken.lexeme, std::move(args));
        }

        // Subregra: incremento/decremento ::= identificador "++" | identificador "--"
        if (match({TipoToken::PlusPLus})) {
            return std::make_unique<IncDecExprNode>(idToken.lexeme, IncDecOp::Increment);
        }
        if (match({TipoToken::MinMin})) {
            return std::make_unique<IncDecExprNode>(idToken.lexeme, IncDecOp::Decrement);
        }

        // Caso base: Identificador puro (Uso de Variável simples)
        return std::make_unique<IdentifierNode>(idToken.lexeme);
    }

    // Se o lookahead não casou com nenhuma regra válida de fator
    throw ParserException("Erro Sintatico: Expressao invalida, operando ou token inesperado.", espiar().posicao);
}