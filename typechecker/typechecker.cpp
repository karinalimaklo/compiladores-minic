#include "typechecker.hpp"
#include <iostream>

//Ponto de entrada principal que inicia o processo de verificação
void TypeChecker::checar(ProgramNode* raiz) {
    if (raiz) {
        //Dispara a caminhada pelo nó raiz da AST
        raiz->accept(this);
    }
}

//Verifica a garantia de return
bool TypeChecker::sempreRetorna(StatementNode* node) {

    if (!node) {
        return false;
    }

    //Caso 1: return
    if (dynamic_cast<ReturnStmtNode*>(node)) {
        return true;
    }

    //Caso 2: bloco de comandos
    if (auto bloco = dynamic_cast<BlockStmtNode*>(node)) {

        for (const auto& comando : bloco->comandos) {

            if (sempreRetorna(comando.get())) {
                return true;
            }
        }

        return false;
    }

    //Caso 3: if/else
    if (auto condicional = dynamic_cast<IfStmtNode*>(node)) {

        //Sem else, existe a possibilidade de não retornar.
        if (!condicional->comandoElse) {
            return false;
        }

        //Os dois caminhos precisam retornar.
        return sempreRetorna(condicional->comandoThen.get()) &&
               sempreRetorna(condicional->comandoElse.get());
    }

    //Caso 4: while
    //Não podemos garantir que o laço será executado.
    if (dynamic_cast<WhileStmtNode*>(node)) {
        return false;
    }

    //Outros comandos não garantem retorno.
    return false;
}

//Valida os Datatypes
void TypeChecker::validarTipoDeclarado(const DataType& tipo, const std::string& contexto, bool permitirVoid) const {

    //Não permite níveis negativos de ponteiros
    if (tipo.pointerLevel < 0) {
        throw std::runtime_error("Erro Semantico: Nivel de ponteiro invalido em " + contexto + ".");
    }
    //Verifica se void está sendo usado sem ponteiro
    if (tipo.kind == DataTypeKind::Void && tipo.pointerLevel == 0 && !permitirVoid) {
        throw std::runtime_error("Erro Semantico: Tipo void nao permitido em " + contexto + ".");
    }
    //Verifica se uma classe/struct utilizada realmente existe
    if (tipo.kind == DataTypeKind::Class) {
        if (m_tabClasses.find(tipo.name) == m_tabClasses.end()) {
            throw std::runtime_error("Erro Semantico: Classe/struct '" + tipo.name + "' nao foi definida em " + contexto + ".");
        }
    }
}

//programa ::= elemento_global* funcao_main
void TypeChecker::visit(ProgramNode* node) {
    
    //Primeira passagem: coleta global de classes
    //Registra todas as classes/structs declaradas globalmente
    for (const auto& classe : node->classes_structs) {
        auto structNode = static_cast<StructClassDeclStmtNode*>(classe.get());
        if (m_tabClasses.find(structNode->nome) != m_tabClasses.end()) {
            throw std::runtime_error("Erro Semantico: Redefinicao de classe/struct '" + structNode->nome + "'.");
        }

        //Cria uma assinatura vazia para a classe
        m_tabClasses.emplace(structNode->nome, ClassSignature{});
    }
    
    //Segunda passagem: coleta atributos e métodos das classes
    for (const auto& classe : node->classes_structs) {
        //Convertemos o nó genérico para o nó específico de declaração
        auto structNode = static_cast<StructClassDeclStmtNode*>(classe.get());
        
        ClassSignature sig;
        //Varre todos os atributos da struct/classe e guarda na assinatura
        for (const auto& campo : structNode->campos) {
             //Valida o tipo do atributo.
            validarTipoDeclarado(campo.tipo,"atributo '" + campo.nome + "'",false);
            if (sig.atributos.find(campo.nome) != sig.atributos.end()) { //checa se a variavel foi redefinida em algum momento
            throw std::runtime_error("Erro Semantico: Redefinicao do atributo '" + campo.nome + "' na classe '" + structNode->nome + "'.");
            }
            //Impede atributo com o mesmo nome de um método.
            if (sig.metodos.find(campo.nome) != sig.metodos.end()) {
                throw std::runtime_error("Erro Semantico: Atributo e metodo possuem o mesmo nome na classe '" + structNode->nome + "'.");
            }
            sig.atributos.emplace(campo.nome, campo.tipo);
        }
        for (const auto& metodo : structNode->metodos) {

            //Valida o tipo de retorno do método
            validarTipoDeclarado(metodo->tipoRetorno, "retorno do metodo '" + metodo->nome + "'", true);
            //Impede métodos duplicados
            if (sig.metodos.find(metodo->nome) != sig.metodos.end()) {
                throw std::runtime_error("Erro Semantico: Redefinicao de metodo '" + metodo->nome + "' na classe '" + structNode->nome + "'.");
            }
            //Impede método com o mesmo nome de um atributo
            if (sig.atributos.find(metodo->nome) != sig.atributos.end()) {
                throw std::runtime_error("Erro Semantico: Atributo e metodo possuem o mesmo nome na classe '" + structNode->nome + "'.");
            }
            
            //Cria a assinatura do método
            FunctionSignature assinatura;
            assinatura.tipoRetorno = metodo->tipoRetorno;

            //Valida e coleta os parâmetros
            for (const auto& param : metodo->parametros) {
                validarTipoDeclarado(param.tipo, "parametro '" + param.nome + "' do metodo '" + metodo->nome + "'", false);
                assinatura.tiposParametros.push_back(param.tipo);
            }
            sig.metodos.emplace(metodo->nome, std::move(assinatura));
        }
        m_tabClasses[structNode->nome] = std::move(sig);
    }

    //Registra todas as funções normais declaradas globalmente
    for (const auto& func : node->funcoes) {
        FunctionSignature sig;
        //Valida o tipo de retorno da função
        validarTipoDeclarado(func->tipoRetorno, "retorno da funcao '" + func->nome + "'", true);
        sig.tipoRetorno = func->tipoRetorno;
        
        //Coleta o tipo de dados de cada parâmetro da assinatura da função
        for (const auto& param : func->parametros) {
            validarTipoDeclarado(param.tipo, "parametro '" + param.nome + "' da funcao '" + func->nome + "'", false);
            sig.tiposParametros.push_back(param.tipo);
        }
        
        if (m_tabFuncoes.find(func->nome) != m_tabFuncoes.end()) {
            throw std::runtime_error("Erro Semantico: Redefinicao da funcao '" + func->nome + "'.");
        }
        m_tabFuncoes[func->nome] = sig;
    }

    //Registra a função obrigatória int main()
    FunctionSignature sigMain;
    sigMain.tipoRetorno = node->funcaoMain->tipoRetorno; //Deve ser Int
    validarTipoDeclarado(node->funcaoMain->tipoRetorno, "retorno da funcao main", false);
    m_tabFuncoes[node->funcaoMain->nome] = sigMain;

    //Checagem do corpo de cada função/main
    //Como o compilador já conhece todas as funções e structs, pode-se entrar no corpo de cada uma para validar os comandos reais
    for (const auto& func : node->funcoes) {
        func->accept(this); //Visita e checa o corpo de cada função individualmente
    }
    for (const auto& classe : node->classes_structs){
        m_tabVariaveis.entrarEscopo();
        auto classe_temp = static_cast<StructClassDeclStmtNode*> (classe.get());
        m_tabVariaveis.inserir("this", DataType{DataTypeKind::Class, classe_temp->nome, 1});
        for(const auto& metodo : classe_temp->metodos) {
            metodo->accept(this);
        }
        m_tabVariaveis.sairEscopo();
    }
    node->funcaoMain->accept(this); //Visita e checa o corpo da main
}

//Visita e verificação de funções
void TypeChecker::visit(FunctionNode* node) {
    //Guarda o tipo de retorno esperado para validar os comandos 'return' lá dentro
    m_tipoRetornoAtual = node->tipoRetorno;

    //Abre um novo escopo para a função (onde ficarão os parâmetros e variáveis locais)
    m_tabVariaveis.entrarEscopo();

    //Insere todos os parâmetros na tabela de símbolos local
    for (const auto& param : node->parametros) {
        //Tenta inserir o parâmetro no escopo atual
        bool inserido = m_tabVariaveis.inserir(param.nome, param.tipo);
        
        //Se retornar false, significa que o usuário declarou dois parâmetros com o mesmo nome (ex: foo(int x, int x))
        if (!inserido) {
            throw std::runtime_error("Erro Semantico: Redefinicao do parametro '" + param.nome + "' na funcao '" + node->nome + "'.");
        }
    }

    //Se a função possuir um corpo, o Visitor checará o bloco recursivamente
    if (node->bloco) {
        node->bloco->accept(this);
    }

    //Verifica se uma função não void garante retorno
    if ((node->tipoRetorno.kind != DataTypeKind::Void ||
        node->tipoRetorno.pointerLevel > 0)) {

        if (!sempreRetorna(node->bloco.get())) {
            throw std::runtime_error("Erro Semantico: Funcao '" + node->nome +"' pode terminar sem retornar um valor.");
        }
    }


    //Limpa o escopo local ao sair da função
    m_tabVariaveis.sairEscopo();
}

//A declaração de classes já foi processada no visit de FunctionNode, então o Visitor não precisa fazer nada aqui
void TypeChecker::visit(StructClassDeclStmtNode* node) {
    //Vazio por design
}

//Resolve o tipo de Literais Atômicos (Ex: 42, 3.14, 'a', true)
void TypeChecker::visit(LiteralNode* node) {
    //O tipo do literal foi anotado pelo parser e é propagado diretamente para cima
    m_tipoExprAtual = node->tipoLiteral;
}

//Resolve o tipo no uso de uma variável simples (Ex: x = 10; ou retornar x;)
void TypeChecker::visit(IdentifierNode* node) {
    //Busca a variável na tabela de símbolos local/global gerenciada por escopos
    auto encontrado = m_tabVariaveis.buscar(node->nome);
    
    //Se a busca retornar std::nullopt, significa que a variável não foi declarada
    if (!encontrado.has_value()) {
        throw std::runtime_error("Erro Semantico: Variavel '" + node->nome + "' nao foi declarada neste escopo.");
    }
    
    //Propaga o tipo correto encontrado na memória para o nó pai
    m_tipoExprAtual = encontrado.value();
}

//Função auxiliar privada para comparar se dois tipos são exatamente iguais
bool tiposSaoIguais(const DataType& t1, const DataType& t2) {
    if (t1.kind == DataTypeKind::Void && t1.pointerLevel == 1) {
        return t2.pointerLevel == 1;
    }
    if (t2.kind == DataTypeKind::Void && t2.pointerLevel == 1) {
        return t1.pointerLevel == 1;
    }
    if (t1.kind != t2.kind) return false;
    if (t1.pointerLevel != t2.pointerLevel) return false;
    if (t1.kind == DataTypeKind::Class && t1.name != t2.name) return false;
    return true;
}

//Resolve o tipo de Operações Binárias (Aritméticas, Relacionais, Igualdade, Lógicas)
void TypeChecker::visit(BinaryExprNode* node) {
    //Avalia o lado esquerdo
    node->esquerda->accept(this);
    DataType tipoEsq = m_tipoExprAtual;

    //Avalia o lado direito
    node->direita->accept(this);
    DataType tipoDir = m_tipoExprAtual;

    //Aplica as regras de checagem com base no operador
    switch (node->op) {
        //Operadores Aritméticos: +, -, *, /, %
        case BinaryOp::Add:
        case BinaryOp::Sub:
        case BinaryOp::Mul:
        case BinaryOp::Div:
        case BinaryOp::Mod: {
            //No MiniC++, operações aritméticas puras exigem que ambos sejam Int ou Double (e iguais entre si)
            if (!tiposSaoIguais(tipoEsq, tipoDir)) {
                throw std::runtime_error("Erro Semantico: Tipos incompativeis na operacao aritmetica.");
            }
            if (tipoEsq.kind != DataTypeKind::Int && tipoEsq.kind != DataTypeKind::Double) {
                throw std::runtime_error("Erro Semantico: Operadores aritmeticos exigem tipos numericos (Int ou Double).");
            }
            if (node->op == BinaryOp::Mod && tipoEsq.kind != DataTypeKind::Int) {
                throw std::runtime_error("Erro Semantico: O operador '%' exige operandos inteiros.");
            }
            if (tipoEsq.pointerLevel > 0) {
                throw std::runtime_error("Erro Semantico: Aritmetica direta de ponteiros nao suportada neste operador.");
            }
            
            //O tipo resultante da expressão matemática é o próprio tipo dos operandos
            m_tipoExprAtual = tipoEsq;
            break;
        }

        //Operadores Relacionais: <, <=, >, >=, ==, !=
        case BinaryOp::LessThan:
        case BinaryOp::LessEqual:
        case BinaryOp::GreaterThan:
        case BinaryOp::GreaterEqual:{
            bool permitido = (tipoEsq.kind == DataTypeKind::Int || tipoEsq.kind == DataTypeKind::Double || tipoEsq.kind == DataTypeKind::Char);
            if (!permitido || tipoEsq.pointerLevel > 0 || !tiposSaoIguais(tipoEsq, tipoDir)) {
            throw std::runtime_error("Erro Semantico: Operadores relacionais exigem operandos numericos compativeis ou char.");
            }
            //O resultado de qualquer comparação relacional ou igualdade é sempre Booleano
            m_tipoExprAtual = DataType{DataTypeKind::Bool, "", 0};
            break;
        }
        case BinaryOp::Equal:
        case BinaryOp::NotEqual: {
            //Ambos os lados precisam ter tipos compatíveis para serem comparados
            if (!tiposSaoIguais(tipoEsq, tipoDir)) {
                throw std::runtime_error("Erro Semantico: Tipos incompativeis na comparacao.");
            }
            
            //O resultado de qualquer comparação relacional ou igualdade é sempre Booleano
            m_tipoExprAtual = DataType{DataTypeKind::Bool, "", 0};
            break;
        }

        //Operadores Lógicos: &&, ||
        case BinaryOp::LogicalAnd:
        case BinaryOp::LogicalOr: {
            //Ambos os lados precisam ser estritamente Booleanos
            if (tipoEsq.kind != DataTypeKind::Bool || tipoDir.kind != DataTypeKind::Bool ||
                tipoEsq.pointerLevel > 0 || tipoDir.pointerLevel > 0) {
                throw std::runtime_error("Erro Semantico: Operadores logicos exigem operandos do tipo Bool.");
            }
            
            m_tipoExprAtual = DataType{DataTypeKind::Bool, "", 0};
            break;
        }
    }
}

//Resolve o tipo de Operações Unárias (Ex: !fator, -fator, &fator, *fator)
void TypeChecker::visit(UnaryExprNode* node) {
    //Avalia recursivamente o fator interno para descobrir o tipo dele
    node->fator->accept(this);
    DataType tipoFator = m_tipoExprAtual;

    switch (node->op) {
        //Negação Lógica: !fator
        case UnaryOp::LogicalNot: {
            if (tipoFator.kind != DataTypeKind::Bool || tipoFator.pointerLevel > 0) {
                throw std::runtime_error("Erro Semantico: O operador '!' exige um operando do tipo Bool.");
            }
            m_tipoExprAtual = tipoFator; //Permanece Bool
            break;
        }

        //Negativo Numérico: -fator
        case UnaryOp::Negate: {
            if ((tipoFator.kind != DataTypeKind::Int && tipoFator.kind != DataTypeKind::Double) || tipoFator.pointerLevel > 0) {
                throw std::runtime_error("Erro Semantico: O operador '-' unario exige um tipo numerico (Int ou Double).");
            }
            m_tipoExprAtual = tipoFator; //Mantém o tipo numérico correspondente
            break;
        }

        //Operador de endereço: &x (Aumenta o nível de ponteiro)
        case UnaryOp::AddressOf: {
            if (!node->fator->isLValue) {throw std::runtime_error("Erro Semantico: O operador '&' exige um LValue.");}
            DataType tipoResultado = tipoFator;
            tipoResultado.pointerLevel++; //Se era Int (0), vira Int* (1). Se era Int* (1), vira Int** (2)
            if (auto idNode = dynamic_cast<IdentifierNode*>(node->fator.get())) {
            if(idNode->nome == "this"){throw std::runtime_error("Erro Semantico: Tentativa de modificação do valor de referência do this.");}
            }
            m_tipoExprAtual = tipoResultado;
            break;
        }

        //Operador de desreferenciação: *p (Diminui o nível de ponteiro)
        case UnaryOp::Deref: {
            if (tipoFator.pointerLevel == 0) {
                throw std::runtime_error("Erro Semantico: Nao eh possivel desreferenciar um tipo que nao eh ponteiro.");
            }
            if (tipoFator.kind == DataTypeKind::Void && tipoFator.pointerLevel == 1) {throw std::runtime_error("Nao e permitido desreferenciar um ponteiro void.");}
            
            DataType tipoResultado = tipoFator;
            tipoResultado.pointerLevel--; //Se era Int* (1), vira Int (0)
            
            m_tipoExprAtual = tipoResultado;
            break;
        }
    }
}

//Resolve o tipo de incremento e decremento (Ex: id++, id--)
void TypeChecker::visit(IncDecExprNode* node) {
    //Busca a variável para garantir que ela existe e descobrir seu tipo
    auto encontrado = m_tabVariaveis.buscar(node->identificador);
    if (!encontrado.has_value()) {
        throw std::runtime_error("Erro Semantico: Variavel '" + node->identificador + "' nao declarada.");
    }

    DataType tipoVar = encontrado.value();

    //Incremento e decremento só fazem sentido em variáveis numéricas puras
    if (tipoVar.kind != DataTypeKind::Int || tipoVar.pointerLevel > 0) {
        throw std::runtime_error("Erro Semantico: Operadores '++' e '--' exigem uma variavel numerica comum.");
    }

    //O tipo resultante da expressão id++ é o próprio tipo da variável
    m_tipoExprAtual = tipoVar;
}

//Resolve o tipo de uma chamada de função (Ex: foo(expr1, expr2))
void TypeChecker::visit(FunctionCallExprNode* node) {
    //Verifica se a função foi declarada globalmente
    auto it = m_tabFuncoes.find(node->nomeFuncao);
    if (it == m_tabFuncoes.end()) {
        throw std::runtime_error("Erro Semantico: Chamada a funcao '" + node->nomeFuncao + "' nao declarada.");
    }

    const FunctionSignature& sig = it->second;

    //Verifica se a quantidade de argumentos fornecida está correta
    if (node->argumentos.size() != sig.tiposParametros.size()) {
        throw std::runtime_error("Erro Semantico: Numero incorreto de argumentos na chamada da funcao '" + node->nomeFuncao + "'.");
    }

    //Avalia e valida o tipo de cada argumento passado recursivamente
    for (size_t i = 0; i < node->argumentos.size(); ++i) {
        node->argumentos[i]->accept(this); //Calcula o tipo do argumento e salva em m_tipoExprAtual
        DataType tipoArg = m_tipoExprAtual;
        DataType tipoParamEsperado = sig.tiposParametros[i];

        if (!tiposSaoIguais(tipoArg, tipoParamEsperado)) {
            throw std::runtime_error("Erro Semantico: Tipo incompativel no argumento " + std::to_string(i + 1) + " da chamada de '" + node->nomeFuncao + "'.");
        }
    }

    //O tipo resultante da chamada inteira é o tipo de retorno que a função promete entregar
    m_tipoExprAtual = sig.tipoRetorno;
}

//Resolve o tipo de um acesso a campo (Ex: objeto.campo ou ponteiro->campo)
void TypeChecker::visit(MemberAccessExprNode* node) {
    //Avalia a expressão do objeto à esquerda para descobrir o tipo dele
    node->objeto->accept(this);
    DataType tipoObjeto = m_tipoExprAtual;

    //Valida o nível de ponteiro de acordo com o operador (. ou ->)
    if (node->isArrow) {
        //Operador -> exige que o objeto seja um ponteiro de nível 1 (ex: Pessoa* p)
        if (tipoObjeto.pointerLevel != 1) {
            throw std::runtime_error("Erro Semantico: O operador '->' só pode ser usado em ponteiros de classe/struct.");
        }
    } else {
        //Operador . exige que o objeto seja um tipo direto (nível 0)
        if (tipoObjeto.pointerLevel != 0) {
            throw std::runtime_error("Erro Semantico: O operador '.' nao pode ser usado em ponteiros. Use '->'.");
        }
    }

    //Garante que o tipo base é de fato uma Classe/Struct cadastrada no sistema
    if (tipoObjeto.kind != DataTypeKind::Class) {
        throw std::runtime_error("Erro Semantico: Tentativa de acessar membro em um tipo primitivo que nao possui atributos.");
    }

    auto itClasse = m_tabClasses.find(tipoObjeto.name);
    if (itClasse == m_tabClasses.end()) {
        throw std::runtime_error("Erro Semantico: Tipo de classe/struct '" + tipoObjeto.name + "' nao foi definido.");
    }

    const ClassSignature& sigClasse = itClasse->second;
    if (!(node->isMetodo)) {
        auto itAtributo = sigClasse.atributos.find(node->nomeMembro);
        if (itAtributo == sigClasse.atributos.end()) { //
            throw std::runtime_error("Erro Semantico: O membro '" + node->nomeMembro + "' nao existe na classe/struct '" + tipoObjeto.name + "'.");
        } 
        m_tipoExprAtual = itAtributo->second;
    }   else {
            auto itMetodo = sigClasse.metodos.find(node->nomeMembro);
                if(itMetodo == sigClasse.metodos.end()) {
                    throw std::runtime_error("Erro Semantico: O membro '" + node->nomeMembro + "' nao existe na classe/struct '" + tipoObjeto.name + "'.");
                } else {
                    const FunctionSignature assinatura = itMetodo->second;
                    if(assinatura.tiposParametros.size() != node->metodo->argumentos.size()) {
                        throw std::runtime_error("Erro Semantico: Numero incorreto de argumentos na chamada do metodo '" + node->metodo->nomeFuncao + "'.");
                    }
                    for (size_t i = 0; i < node->metodo->argumentos.size(); ++i) {
                        node->metodo->argumentos[i]->accept(this); //Calcula o tipo do argumento e salva em m_tipoExprAtual
                        DataType tipoArg = m_tipoExprAtual;
                        DataType tipoParamEsperado = assinatura.tiposParametros[i];

                        if (!tiposSaoIguais(tipoArg, tipoParamEsperado)) {
                            throw std::runtime_error("Erro Semantico: Tipo incompativel no argumento " + std::to_string(i + 1) + " da chamada de '" + node->metodo->nomeFuncao + "'.");
                        }
                }
                m_tipoExprAtual = itMetodo->second.tipoRetorno;

        }
    }
}
    //Verifica se o campo/atributo existe dentro da definição da classe
    //O tipo resultante da expressão inteira de acesso é o tipo do próprio campo consultado


// Métodos de visita para comandos/statements (Apenas validam regras)

//Valida um bloco fechado de comandos: bloco ::= "{" comando* "}"
void TypeChecker::visit(BlockStmtNode* node) {
    //Abre um novo escopo local na tabela de símbolos para este bloco de chaves
    m_tabVariaveis.entrarEscopo();

    //Visita recursivamente cada comando de dentro do bloco
    for (const auto& comando : node->comandos) {
        if (comando) {
            comando->accept(this);
        }
    }

    //Destrói o escopo local ao sair do bloco, limpando as variáveis criadas aqui dentro
    m_tabVariaveis.sairEscopo();
}

//Valida um comando de expressão descartada (Ex: id++; ou foo();)
void TypeChecker::visit(ExpressionStmtNode* node) {
    if (node->expressao) {
        //Apenas dispara a avaliação da expressão interna para garantir que ela não possui erros de tipo
        node->expressao->accept(this);
    }
}

//Valida o comando 'break' garantindo que ele está dentro de um laço
void TypeChecker::visit(BreakStmtNode* node) {
    if (m_profundidadeLacos == 0) {
        throw std::runtime_error("Erro Semantico: O comando 'break' so pode ser usado dentro de um laco (while/for).");
    }
}

//Valida o comando 'continue' garantindo que ele está dentro de um laço
void TypeChecker::visit(ContinueStmtNode* node) {
    if (m_profundidadeLacos == 0) {
        throw std::runtime_error("Erro Semantico: O comando 'continue' so pode ser usado dentro de um laco (while/for).");
    }
}

//Valida declarações de variáveis locais: declaracao_variavel ::= tipo_basico declarador ("," declarador)* ";"
void TypeChecker::visit(VariableDeclStmtNode* node) {

    //Se o tipo básico for uma Classe/Struct, verifica se ela existe na tabela global
    //Ultimo argumento é false porque variável não pode ser declarada como void puro
    validarTipoDeclarado(node->tipoBasico, "declaracao de variavel", false);

    //Processa cada um dos declaradores da linha (Ex: int a = 2, b;)
    for (const auto& decl : node->declaradores) {
        
        //Se houver uma expressão de inicialização (Ex: = 10)
        if (decl.inicializador) {
            decl.inicializador->accept(this); //Calcula o tipo do inicializador em m_tipoExprAtual
            DataType tipoInicializador = m_tipoExprAtual;

            //Valida se o tipo do inicializador bate com o tipo que a variável declarou
            if (!tiposSaoIguais(node->tipoBasico, tipoInicializador)) {
                throw std::runtime_error("Erro Semantico: Tipo incompativel na inicializacao da variavel '" + decl.nome + "'.");
            }
        }

        //Insere a nova variável com o seu tipo no escopo atual da tabela de símbolos
        bool inserido = m_tabVariaveis.inserir(decl.nome, node->tipoBasico);
        if (!inserido) {
            throw std::runtime_error("Erro Semantico: Redefinicao da variavel '" + decl.nome + "' no mesmo escopo.");
        }
    }
}

//Valida a atribuição de valores: atribuicao ::= identificador "=" expressao ";"
void TypeChecker::visit(AssignmentStmtNode* node) {
    if (!node->esquerda->isLValue) {throw std::runtime_error("Erro Semantico: Lado esquerdo da atribuicao nao eh um LValue.");}
    //Avalia o lado esquerdo da atribuição recursivamente através do Visitor
    node->esquerda->accept(this);
    DataType tipoEsquerda = m_tipoExprAtual;
    if (auto nodeLeft = dynamic_cast<MemberAccessExprNode*>(node->esquerda.get())) {
        if(nodeLeft->isMetodo) {
            throw std::runtime_error("Erro Semantico: tentou gravar um valor em um método.");
        } 
    } 
    //Avalia o lado direito da atribuição
    node->direita->accept(this);
    DataType tipoDireita = m_tipoExprAtual;
    if(auto nodeLeft = dynamic_cast<IdentifierNode*>(node->esquerda.get())) {
        if(nodeLeft->nome == "this") {
            throw std::runtime_error("Erro Semantico: tentou gravar um valor na referência *this*");
        }
    }
    //Verifica se os tipos da esquerda e da direita são exatamente iguais na matemática da linguagem
    if (!tiposSaoIguais(tipoEsquerda, tipoDireita)) {
        throw std::runtime_error("Erro Semantico: Tipos incompativeis na atribuicao.");
    }
}

//Valida comandos condicionais: comando_if ::= "if" "(" expressao ")" comando ("else" comando)?
void TypeChecker::visit(IfStmtNode* node) {
    //Avalia a expressão da condição
    node->condicao->accept(this);
    if (m_tipoExprAtual.kind != DataTypeKind::Bool || m_tipoExprAtual.pointerLevel > 0) {
        throw std::runtime_error("Erro Semantico: A condicao do comando 'if' deve ser do tipo Bool.");
    }

    //Avalia o bloco ou comando do 'then'
    if (node->comandoThen) {
        node->comandoThen->accept(this);
    }

    //Avalia o bloco ou comando do 'else'
    if (node->comandoElse) {
        node->comandoElse->accept(this);
    }
}

//Valida laços de repetição: comando_while ::= "while" "(" expressao ")" comando
void TypeChecker::visit(WhileStmtNode* node) {
    //Avalia a expressão da condição do laço
    node->condicao->accept(this);
    if (m_tipoExprAtual.kind != DataTypeKind::Bool || m_tipoExprAtual.pointerLevel > 0) {
        throw std::runtime_error("Erro Semantico: A condicao do comando 'while' deve ser do tipo Bool.");
    }

    //Ativa o controle de escopo do break/continue incrementando o contador
    m_profundidadeLacos++;

    //Avalia o corpo do laço
    if (node->comandoBody) {
        node->comandoBody->accept(this);
    }

    //Decrementa o contador ao sair do escopo do laço
    m_profundidadeLacos--;
}

//Valida o comando de retorno: comando_return ::= "return" expressao? ";"
void TypeChecker::visit(ReturnStmtNode* node) {
    //Caso 1: O comando de retorno possui uma expressão (Ex: return x + 1;)
    if (node->expressao) {
        //Se a função atual foi declarada como Void, ela não pode retornar nada
        if (m_tipoRetornoAtual.kind == DataTypeKind::Void && m_tipoRetornoAtual.pointerLevel == 0) {
            throw std::runtime_error("Erro Semantico: Tentativa de retornar um valor em uma funcao com retorno Void.");
        }

        //Avalia o tipo da expressão de retorno
        node->expressao->accept(this);
        DataType tipoRetornado = m_tipoExprAtual;

        //Verifica se o tipo da expressão bate com o que a assinatura da função exige
        if (!tiposSaoIguais(m_tipoRetornoAtual, tipoRetornado)) {
            throw std::runtime_error("Erro Semantico: O tipo retornado nao corresponde ao tipo de retorno assinado pela funcao.");
        }
    } 
    //Caso 2: O comando de retorno está vazio (Ex: return;)
    else {
        //Se a função exige um retorno (como int ou bool), o comando 'return;' vazio é inválido
        if (m_tipoRetornoAtual.kind != DataTypeKind::Void || m_tipoRetornoAtual.pointerLevel > 0) {
            throw std::runtime_error("Erro Semantico: Esperada uma expressao de retorno para esta funcao.");
        }
    }
}
