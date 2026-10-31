#include <vector>
#include "parser.hpp"
#include "../lexer/token.hpp"

Parser::Parser(std::vector<Token> tokens) 
    : m_tokens(std::move(tokens)), m_atual(0) {}

Token Parser::espiar() const {
    return m_tokens[m_atual];
}

Token Parser::anterior() const {
    return m_tokens[m_atual - 1];
}

bool Parser::fim() const {
    return espiar().tipo == TipoToken::FimArquivo;
}

bool Parser::checar(TipoToken tipo) const {
    if (fim()) return false;
    return espiar().tipo == tipo;
}

Token Parser::avancar() {
    if (!fim()) m_atual++;
    return anterior();
}

Token Parser::consumir(TipoToken tipo, const std::string& mensagemErro) {
    if (checar(tipo)) return avancar();
    throw ParserException(mensagemErro, espiar().posicao);
}

bool Parser::match(const std::vector<TipoToken>& tipos) {
    for (TipoToken tipo : tipos) {
        if (checar(tipo)) {
            avancar();
            return true;
        }
    }
    return false;
}

DataType Parser::mapearTipoBasico() {
    DataType tipo;
    
    //Identifica a base do tipo (tipo_base ::= "int" | "bool" | "char" | "double" | identificador)
    if (checar(TipoToken::PalInt)) {
        tipo.kind = DataTypeKind::Int;
        avancar();
    } else if (checar(TipoToken::PalBool)) {
        tipo.kind = DataTypeKind::Bool;
        avancar();
    } else if (checar(TipoToken::PalChar)) {
        tipo.kind = DataTypeKind::Char;
        avancar();
    } else if (checar(TipoToken::PalDouble)) {
        tipo.kind = DataTypeKind::Double;
        avancar();
    } else if (checar(TipoToken::PalVoid)) {
        tipo.kind = DataTypeKind::Void;
        avancar();
    } else if (checar(TipoToken::Ident)) {
        //Se for um identificador no lugar de um tipo, significa que é uma Classe/Struct
        tipo.kind = DataTypeKind::Class;
        tipo.name = avancar().lexeme; //Guarda o nome da classe (ex: "Pessoa")
    } else {
        throw ParserException("Erro Sintatico: Tipo de dado invalido ou nao suportado.", espiar().posicao);
    }

    //Captura o nível do ponteiro (Consome todos os '*' seguidos, ex: int** -> level 2) (ponteiro_opt ::= ("*")*)
    while (checar(TipoToken::Star)) {
        avancar();
        tipo.pointerLevel++;
    }

    return tipo;
}

//Processa a lista de parâmetros: parametro ("," parametro)*
std::vector<Parameter> Parser::parseParametros() {
    std::vector<Parameter> lista;
    lista.push_back(parseParametro());
    
    while (match({TipoToken::Virg})) {
        lista.push_back(parseParametro());
    }
    return lista;
}

//Processa um único parâmetro: tipo_basico identificador
Parameter Parser::parseParametro() {
    DataType tipo = mapearTipoBasico();
    Token idToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado identificador para o parametro.");
    
    return Parameter{tipo, idToken.lexeme};
}

//Processa os argumentos passados em chamadas: argumentos ::= expressao ("," expressao)*
std::vector<std::unique_ptr<ExpressionNode>> Parser::parseArgumentos() {
    std::vector<std::unique_ptr<ExpressionNode>> lista;
    lista.push_back(parseExpressao());
    
    while (match({TipoToken::Virg})) {
        lista.push_back(parseExpressao());
    }
    return lista;
}

//Ponto de entrada principal: programa ::= elemento_global* funcao_main
std::unique_ptr<ProgramNode> Parser::parse() {
    std::vector<std::unique_ptr<StatementNode>> classes;
    std::vector<std::unique_ptr<FunctionNode>> funcoes;
    std::unique_ptr<FunctionNode> funcaoMain = nullptr;

    while (!fim()) {

        //Se encontrar uma classe ou struct na raiz do arquivo
        if (checar(TipoToken::PalClass) || checar(TipoToken::PalStruct)) {
            classes.push_back(parseDeclaracaoStructClasse()); 
            continue;
        }

        //Se o tipo for 'int' e o proximo token for 'main', tem que ser a main
        if (checar(TipoToken::PalInt) && 
            (m_atual + 1 < m_tokens.size() && m_tokens[m_atual + 1].lexeme == "main")) {
            funcaoMain = parseFuncaoMain();
            break; //A main é o último elemento do arquivo
        } else {
            funcoes.push_back(parseDeclaracaoFuncao());
        }
    }

    if (!funcaoMain) {
        throw ParserException("Erro Sintatico: Funcao obrigatoria 'int main()' nao encontrada ou fora de posicao.", espiar().posicao);
    }

    return std::make_unique<ProgramNode>(std::move(classes), std::move(funcoes), std::move(funcaoMain));
}

//Processa uma função comum: declaracao_funcao ::= tipo_retorno identificador "(" parametros? ")" bloco
std::unique_ptr<FunctionNode> Parser::parseDeclaracaoFuncao() {
    DataType tipoRet = mapearTipoBasico();
    Token idToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado identificador com o nome da funcao.");
    
    consumir(TipoToken::ColEsquerda, "Erro Sintatico: Esperado '(' apos o nome da funcao.");
    std::vector<Parameter> params;
    if (!checar(TipoToken::ColDireita)) {
        params = parseParametros();
    } //lembrar de fazer mudança
    consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos a lista de parametros.");

    //Chama o analisador de bloco de comandos: bloco ::= "{" comando* "}"
    std::unique_ptr<BlockStmtNode> corpo = parseBloco();

    return std::make_unique<FunctionNode>(tipoRet, idToken.lexeme, std::move(params), std::move(corpo));
}

// Processa a declaração de uma Struct ou Classe: declaracao_struct_classe ::= ("struct" | "class") identificador "{" campo_atributo* "}" ";"
std::unique_ptr<StatementNode> Parser::parseDeclaracaoStructClasse() {
    //Identifica se é class ou struct e consome o token correspondente
    bool isClass = false;
    if (match({TipoToken::PalClass})) {
        isClass = true;
    } else {
        consumir(TipoToken::PalStruct, "Erro Sintatico: Esperado 'class' ou 'struct'.");
    }

    //Consome o nome da Classe/Struct
    Token idToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado nome para a classe ou struct.");

    //Abre o escopo interno de atributos
    consumir(TipoToken::ChavesEsq, "Erro Sintatico: Esperado '{' para iniciar o corpo da classe ou struct.");
    std::vector<FieldDeclarator> campos;

    //Loop para ler todos os atributos internos até encontrar o fechamento de '}'
    while (!checar(TipoToken::ChavesDir) && !fim()) {
        //Coleta o tipo do atributo (primitivo, ponteiro ou outra classe)
        DataType tipoMembro = mapearTipoBasico();
        
        //Coleta o nome do atributo
        Token nomeMembroToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado nome para o atributo.");
        consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos o nome do atributo.");
        
        campos.push_back(FieldDeclarator{tipoMembro, nomeMembroToken.lexeme});
    }

    consumir(TipoToken::ChavesDir, "Erro Sintatico: Esperado '}' para fechar o corpo da classe ou struct.");
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos o fechamento da classe ou struct.");

    return std::make_unique<StructClassDeclStmtNode>(idToken.lexeme, std::move(campos), isClass);
}


//Processa especificamente a main: funcao_main ::= "int" "main" "(" ")" bloco
std::unique_ptr<FunctionNode> Parser::parseFuncaoMain() {
    consumir(TipoToken::PalInt, "Erro Sintatico: O tipo de retorno da funcao main deve ser obrigatoriamente 'int'.");
    
    Token mainId = consumir(TipoToken::Ident, "Erro Sintatico: Esperado identificador 'main'.");
    if (mainId.lexeme != "main") {
        throw ParserException("Erro Sintatico: Identificador inválido para a funcao principal, esperado 'main'.", mainId.posicao);
    }

    consumir(TipoToken::ColEsquerda, "Erro Sintatico: Esperado '(' na assinatura da funcao main.");
    consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' na assinatura da funcao main.");

    std::unique_ptr<BlockStmtNode> corpo = parseBloco();

    //Cria o tipo int para o retorno da main usando a struct nova
    DataType tipoRetornoMain{DataTypeKind::Int, "", 0};
    return std::make_unique<FunctionNode>(tipoRetornoMain, "main", std::vector<Parameter>{}, std::move(corpo));
}

//Processa um bloco fechado: bloco ::= "{" comando* "}"
std::unique_ptr<BlockStmtNode> Parser::parseBloco() {
    consumir(TipoToken::ChavesEsq, "Erro Sintatico: Esperado '{' para iniciar o bloco de comandos.");
    std::vector<std::unique_ptr<StatementNode>> comandos;
    
    //Continua consumindo comandos até encontrar o fechamento ou o fim do arquivo
    while (!checar(TipoToken::ChavesDir) && !fim()) {
        comandos.push_back(parseComando());
    }
    
    consumir(TipoToken::ChavesDir, "Erro Sintatico: Esperado '}' para fechar o bloco de comandos.");
    return std::make_unique<BlockStmtNode>(std::move(comandos));
}

//Processa condicionais: comando_if ::= "if" "(" expressao ")" comando ("else" comando)?
std::unique_ptr<StatementNode> Parser::parseComandoIf() {
    avancar(); //Consome o token 'if'
    consumir(TipoToken::ColEsquerda, "Erro Sintatico: Esperado '(' apos a palavra-chave 'if'.");
    std::unique_ptr<ExpressionNode> condicao = parseExpressao();
    consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos a condicao do 'if'.");
    
    std::unique_ptr<StatementNode> comandoThen = parseComando();
    std::unique_ptr<StatementNode> comandoElse = nullptr;

    //Verifica a presença opcional do 'else'
    if (match({TipoToken::PalElse})) {
        comandoElse = parseComando();
    }

    return std::make_unique<IfStmtNode>(std::move(condicao), std::move(comandoThen), std::move(comandoElse));
}

//Processa laços: comando_while ::= "while" "(" expressao ")" comando
std::unique_ptr<StatementNode> Parser::parseComandoWhile() {
    avancar(); //Consome o token 'while'
    consumir(TipoToken::ColEsquerda, "Erro Sintatico: Esperado '(' apos a palavra-chave 'while'.");
    std::unique_ptr<ExpressionNode> condicao = parseExpressao();
    consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos a condicao do 'while'.");
    
    std::unique_ptr<StatementNode> comandoBody = parseComando();
    return std::make_unique<WhileStmtNode>(std::move(condicao), std::move(comandoBody));
}

//Processa quebras de laço: comando_break ::= "break" ";"
std::unique_ptr<StatementNode> Parser::parseComandoBreak() {
    avancar(); //Consome o token 'break'
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos o comando 'break'.");
    return std::make_unique<BreakStmtNode>();
}

//Processa continuação de laço: comando_continue ::= "continue" ";"
std::unique_ptr<StatementNode> Parser::parseComandoContinue() {
    avancar(); //Consome o token 'continue'
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos o comando 'continue'.");
    return std::make_unique<ContinueStmtNode>();
}

//Processa o retorno: comando_return ::= "return" expressao? ";"
std::unique_ptr<StatementNode> Parser::parseComandoReturn() {
    avancar(); //Consome o token 'return'
    std::unique_ptr<ExpressionNode> expressao = nullptr;
    
    //Se o próximo token não for um ponto e vírgula, significa que há uma expressão de retorno
    if (!checar(TipoToken::PontoVirg)) {
        expressao = parseExpressao();
    }
    
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos o comando 'return'.");
    return std::make_unique<ReturnStmtNode>(std::move(expressao));
}

//Processa os comandos: comando ::= declaracao_variavel | atribuicao | comando_if | ...
std::unique_ptr<StatementNode> Parser::parseComando() {
    //Se começar com uma palavra-chave de tipo básico, é uma declaração de variável
    if (checar(TipoToken::PalInt) || checar(TipoToken::PalBool) || 
        checar(TipoToken::PalChar) || checar(TipoToken::PalDouble)){
        return parseDeclaracaoVariavel();
    }

    //Resolve a ambiguidade do identificador (Classe vs Variável/Função)
    if (checar(TipoToken::Ident)) {
        bool ehDeclaracao = false;
        
        //Verifica se há tokens suficientes à frente para espiar
        if (m_atual + 1 < m_tokens.size()) {
            TipoToken proximo = m_tokens[m_atual + 1].tipo;
            
            //Se for "Ident Ident" (ex: Pessoa p;) ou "Ident *" (ex: Pessoa* p;)
            if (proximo == TipoToken::Ident || proximo == TipoToken::Star) {
                ehDeclaracao = true;
            }
        }
        
        if (ehDeclaracao) {
            return parseDeclaracaoVariavel();
        }
    }
    
    //Encaminha para os comandos de controle correspondentes
    if (checar(TipoToken::PalIf))       return parseComandoIf();
    if (checar(TipoToken::PalWhile))    return parseComandoWhile();
    if (checar(TipoToken::PalBreak))    return parseComandoBreak();
    if (checar(TipoToken::PalContinue)) return parseComandoContinue();
    if (checar(TipoToken::PalRet))      return parseComandoReturn();
    if (checar(TipoToken::ChavesEsq))   return parseBloco();

    //Se não cair em nenhuma das opções acima, restam duas possibilidades da gramática:
    //Uma atribuição (id = expr;) ou uma expressão pura terminada em ';' (ex: id++; ou chamadas de função)
    return parseAtribuicaoOuExpressaoStmt();
}

//Processa variáveis: declaracao_variavel ::= tipo_basico declarador ("," declarador)* ";"
std::unique_ptr<StatementNode> Parser::parseDeclaracaoVariavel() {
    //Coleta o tipo básico (int, bool, char, double) e avança
    DataType tipo = mapearTipoBasico();
    std::vector<VariableDeclarator> dees;

    //Processa pelo menos um declarador, repetindo enquanto houver vírgula
    do {
        Token idToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado nome da variavel.");
        std::unique_ptr<ExpressionNode> inicializador = nullptr;
        
        //Verifica se há uma inicialização opcional (ex: = 10)
        if (match({TipoToken::Assign})) {
            inicializador = parseExpressao();
        }
        
        dees.push_back(VariableDeclarator{idToken.lexeme, std::move(inicializador)});
    } while (match({TipoToken::Virg}));

    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' apos a declaracao de variavel(is).");
    return std::make_unique<VariableDeclStmtNode>(tipo, std::move(dees));
}

//Resolve a ambiguidade do espiar: atribuição (id = expr;) e expressão isolada (id++; ou foo();) (comando ::= atribuicao e comando ::= expressao ";")
std::unique_ptr<StatementNode> Parser::parseAtribuicaoOuExpressaoStmt() {
    //Faz o parsing da expressão da esquerda normalmente (consome "x", "*p", "v.x" ou "ptr->x")
    std::unique_ptr<ExpressionNode> exprEsq = parseExpressao();

    //Se o próximo token for um '='
    if (match({TipoToken::Assign})) {
        //Faz o parsing da expressão do lado direito
        std::unique_ptr<ExpressionNode> exprDir = parseExpressao();
        consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' ao final da atribuicao.");
        
        //Cria o nó passando as duas expressões completas
        return std::make_unique<AssignmentStmtNode>(std::move(exprEsq), std::move(exprDir));
    }

    //Se não veio um '=', era apenas uma expressão pura (ex: id++; ou foo();)
    consumir(TipoToken::PontoVirg, "Erro Sintatico: Esperado ';' ao final da expressao.");
    return std::make_unique<ExpressionStmtNode>(std::move(exprEsq));
}

//Ponto de entrada das expressões: expressao ::= expressao_or
std::unique_ptr<ExpressionNode> Parser::parseExpressao() {
    return parseExpressaoOr();
}

//Processa o operador OU lógico: expressao_or ::= expressao_and ( op_logico_or expressao_and )*
std::unique_ptr<ExpressionNode> Parser::parseExpressaoOr() {
    std::unique_ptr<ExpressionNode> expr = parseExpressaoAnd();
    
    //Transforma a recursão à esquerda da gramática em um laço
    while (match({TipoToken::OrOr})) {
        std::unique_ptr<ExpressionNode> direita = parseExpressaoAnd();
        expr = std::make_unique<BinaryExprNode>(BinaryOp::LogicalOr, std::move(expr), std::move(direita));
    }
    return expr;
}

//Processa o operador E lógico: expressao_and ::= expressao_igualdade ( op_logico_and expressao_igualdade )*
std::unique_ptr<ExpressionNode> Parser::parseExpressaoAnd() {
    std::unique_ptr<ExpressionNode> expr = parseExpressaoIgualdade();
    
    while (match({TipoToken::AndAnd})) {
        std::unique_ptr<ExpressionNode> direita = parseExpressaoIgualdade();
        expr = std::make_unique<BinaryExprNode>(BinaryOp::LogicalAnd, std::move(expr), std::move(direita));
    }
    return expr;
}

//Processa igualdades: expressao_igualdade ::= expressao_relacional ( op_igualdade expressao_relacional )*
std::unique_ptr<ExpressionNode> Parser::parseExpressaoIgualdade() {
    std::unique_ptr<ExpressionNode> expr = parseExpressaoRelacional();
    
    while (checar(TipoToken::Eq) || checar(TipoToken::NotEq)) {
        Token opToken = avancar(); //Consome o operador
        BinaryOp op = (opToken.tipo == TipoToken::Eq) ? BinaryOp::Equal : BinaryOp::NotEqual;
        
        std::unique_ptr<ExpressionNode> direita = parseExpressaoRelacional();
        expr = std::make_unique<BinaryExprNode>(op, std::move(expr), std::move(direita));
    }
    return expr;
}

//Processa relacionais: expressao_relacional ::= expr_aritmetica ( op_relacional expr_aritmetica )*
std::unique_ptr<ExpressionNode> Parser::parseExpressaoRelacional() {
    std::unique_ptr<ExpressionNode> expr = parseExprAritmetica();
    
    while (checar(TipoToken::Lt) || checar(TipoToken::Le) || checar(TipoToken::Gt) || checar(TipoToken::Ge)) {
        Token opToken = avancar(); //Consome o operador relacional
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

//Processa somas e subtrações: expr_aritmetica ::= termo ( op_aditivo termo )*
std::unique_ptr<ExpressionNode> Parser::parseExprAritmetica() {
    std::unique_ptr<ExpressionNode> expr = parseTermo();
    
    while (checar(TipoToken::Plus) || checar(TipoToken::Min)) {
        Token opToken = avancar(); //Consome '+' ou '-'
        BinaryOp op = (opToken.tipo == TipoToken::Plus) ? BinaryOp::Add : BinaryOp::Sub;
        
        std::unique_ptr<ExpressionNode> direita = parseTermo();
        expr = std::make_unique<BinaryExprNode>(op, std::move(expr), std::move(direita));
    }
    return expr;
}

//Processa multiplicações, divisões e restos: termo ::= fator ( op_multiplicativo fator )*
std::unique_ptr<ExpressionNode> Parser::parseTermo() {
    std::unique_ptr<ExpressionNode> expr = parseFator();
    
    while (checar(TipoToken::Star) || checar(TipoToken::Div) || checar(TipoToken::Percent)) {
        Token opToken = avancar(); //Consome '*', '/' ou '%'
        BinaryOp op;
        
        if (opToken.tipo == TipoToken::Star)      op = BinaryOp::Mul;
        else if (opToken.tipo == TipoToken::Div) op = BinaryOp::Div;
        else                                     op = BinaryOp::Mod;

        std::unique_ptr<ExpressionNode> direita = parseFator();
        expr = std::make_unique<BinaryExprNode>(op, std::move(expr), std::move(direita));
    }
    return expr;
}

// Processa fatores atômicos e cadeias de acesso: fator ::= fator_nucleo (op_acesso identificador)*
std::unique_ptr<ExpressionNode> Parser::parseFator() {
    //Literais atômicos
    if (match({TipoToken::IntLit})) {
        return std::make_unique<LiteralNode>(DataType{DataTypeKind::Int, "", 0}, anterior().lexeme);
    }
    if (match({TipoToken::DoubleLit})) {
        return std::make_unique<LiteralNode>(DataType{DataTypeKind::Double, "", 0}, anterior().lexeme);
    }
    if (match({TipoToken::CharLit})) {
        return std::make_unique<LiteralNode>(DataType{DataTypeKind::Char, "", 0}, anterior().lexeme);
    }
    if (match({TipoToken::PalTrue})) {
        return std::make_unique<LiteralNode>(DataType{DataTypeKind::Bool, "", 0}, "true");
    }
    if (match({TipoToken::PalFalse})) {
        return std::make_unique<LiteralNode>(DataType{DataTypeKind::Bool, "", 0}, "false");
    }
    if (match({TipoToken::PalNullptr})) {
        return std::make_unique<LiteralNode>(DataType{DataTypeKind::Void, "", 1}, "nullptr");
    }

    //Operadores unários
    if (match({TipoToken::Not})) {
        std::unique_ptr<ExpressionNode> fat = parseFator();
        return std::make_unique<UnaryExprNode>(UnaryOp::LogicalNot, std::move(fat));
    }
    if (match({TipoToken::Min})) { //Números negativos (ex: -5)
        std::unique_ptr<ExpressionNode> fat = parseFator();
        return std::make_unique<UnaryExprNode>(UnaryOp::Negate, std::move(fat));
    }
    if (match({TipoToken::Address})) { 
        std::unique_ptr<ExpressionNode> fat = parseFator();
        return std::make_unique<UnaryExprNode>(UnaryOp::AddressOf, std::move(fat));
    }
    if (match({TipoToken::Star})) { 
        std::unique_ptr<ExpressionNode> fat = parseFator();
        return std::make_unique<UnaryExprNode>(UnaryOp::Deref, std::move(fat));
    }
    //Expressões: "(" expressao ")"
    if (match({TipoToken::ColEsquerda})) {
        std::unique_ptr<ExpressionNode> expr = parseExpressao();
        consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos a expressao aninhada.");
        return expr;
    }

     //Casos que iniciam com identificador (Variavel, chamada ou incremento)
    if (match({TipoToken::Ident})) {
        Token idToken = anterior();
        std::unique_ptr<ExpressionNode> nodoBase = nullptr;

        //Subregra: chamada_funcao ::= identificador "(" argumentos? ")"
        if (match({TipoToken::ColEsquerda})) {
            std::vector<std::unique_ptr<ExpressionNode>> args;
            if (!checar(TipoToken::ColDireita)) {
                args = parseArgumentos();
            }
            consumir(TipoToken::ColDireita, "Erro Sintatico: Esperado ')' apos os argumentos da funcao.");
            nodoBase = std::make_unique<FunctionCallExprNode>(idToken.lexeme, std::move(args));
        }
        //Subregra: incremento ::= identificador "++" | identificador "--"
        else if (match({TipoToken::PlusPLus})) {
            nodoBase = std::make_unique<IncDecExprNode>(idToken.lexeme, IncDecOp::Increment);
        }
        else if (match({TipoToken::MinMin})) {
            nodoBase = std::make_unique<IncDecExprNode>(idToken.lexeme, IncDecOp::Decrement);
        }
        //Caso base: Identificador puro (Uso de variável simples)
        else {
            nodoBase = std::make_unique<IdentifierNode>(idToken.lexeme);
        }

        //Após resolver o núcleo do fator (variável, chamada ou incremento),
        //passamos ele pelo filtro de acesso a membros para checar se há '.' ou '->' conjugados!
        return parseAcessoMembro(std::move(nodoBase));
    }

    //Se o espiar não casou com nenhuma regra válida de fator
    throw ParserException("Erro Sintatico: Expressao invalida, operando ou token inesperado.", espiar().posicao);
}

//Processa a cadeia em loop de acessos: fator ::= fator_nucleo (op_acesso identificador)*
std::unique_ptr<ExpressionNode> Parser::parseAcessoMembro(std::unique_ptr<ExpressionNode> objeto) {
    std::unique_ptr<ExpressionNode> expr = std::move(objeto);

    //Enquanto o próximo token for um ponto '.' ou uma seta '->'
    while (checar(TipoToken::Ponto) || checar(TipoToken::Seta)) {
        Token opToken = avancar(); // Consome o '.' ou '->'
        bool isArrow = (opToken.tipo == TipoToken::Seta);

        //O que vem obrigatoriamente depois do operador é o nome do atributo/campo
        Token membroToken = consumir(TipoToken::Ident, "Erro Sintatico: Esperado nome do membro apos o operador de acesso.");

        //Cria o nó de acesso empilhando a expressão anterior como a subárvore da esquerda
        expr = std::make_unique<MemberAccessExprNode>(std::move(expr), membroToken.lexeme, isArrow);
    }

    return expr;
}
