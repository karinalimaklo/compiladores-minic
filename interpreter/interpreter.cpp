#include "interpreter.hpp"
#include <iostream>

void Interpreter::executar(ProgramNode* raiz) {
    if(raiz) {
        raiz->accept(this);
    }
}

//Inicialização: cadastra definições globais e chama a função main()
void Interpreter::visit(ProgramNode* node) {
    //cadastra todas as classes/structs disponíveis
    for(const auto& classe : node->classes_structs) {
        auto structNode = static_cast<StructClassDeclStmtNode*>(classe.get());
        m_classes[structNode->nome] = structNode;
    }

    //cadastra as funções do programa
    for(const auto& func : node->funcoes) {
        m_funcoes[func->nome] = func.get();
    }

    //executa a função principal main
    if(node->funcaoMain) {
        node->funcaoMain->accept(this);
    }
}

//Executa o corpo de uma função e captura o comando return
void Interpreter::visit(FunctionNode* node) {
    try {
        if(node->bloco) {
            node->bloco->accept(this);
        }
    } catch (const ReturnException& ret) {
        m_ultimoValorExpr = ret.valor;
    }
}

void Interpreter::visit(StructClassDeclStmtNode* node) {
    //Declarações estáticas de estrutura não executam código dinâmico
}

//Avalia literais numéricos, caracteres e booleanos
void Interpreter::visit(LiteralNode* node) {
    switch(node->tipoLiteral.kind) {
        case DataTypeKind::Int:
            m_ultimoValorExpr = std::stoi(node->valor);
            break;
        case DataTypeKind::Double:
            m_ultimoValorExpr = std::stod(node->valor);
            break;
        case DataTypeKind::Char: {
            //remove aspas simples se o lexer as manteve
            std::string val = node->valor;
            if(val.length() >= 3 && val.front() == '\'' && val.back() == '\'') {
                m_ultimoValorExpr = val[1];
            } else {
                m_ultimoValorExpr = val.empty() ? '\0' : val[0];
            }
            break;
        }
        case DataTypeKind::Bool:
            m_ultimoValorExpr = (node->valor == "true");
            break;
        case DataTypeKind::Void:
            if (node->valor == "nullptr") {
                m_ultimoValorExpr = std::shared_ptr<ObjectValue>(nullptr);
            }
            break;
        default:
            break;
    }
}

//Busca o valor da variável armazenada no escopo
void Interpreter::visit(IdentifierNode* node) {
    m_ultimoValorExpr = m_env.obter(node->nome);
}

//Avalia operações aritméticas, relacionais e lógicas
void Interpreter::visit(BinaryExprNode* node) {
    node->esquerda->accept(this);
    Value esq = m_ultimoValorExpr;

    node->direita->accept(this);
    Value dir = m_ultimoValorExpr;

    //Operações com números inteiros
    if(std::holds_alternative<int>(esq) && std::holds_alternative<int>(dir)) {
        int vEsq = std::get<int>(esq);
        int vDir = std::get<int>(dir);

        switch(node->op) {
            case BinaryOp::Add:
                m_ultimoValorExpr = vEsq + vDir;
                break;
            case BinaryOp::Sub:
                m_ultimoValorExpr = vEsq - vDir;
                break;
            case BinaryOp::Mul:
                m_ultimoValorExpr = vEsq * vDir;
                break;
            case BinaryOp::Div:
                if(vDir == 0) {throw RuntimeException("Erro de Execucao: Divisao por zero.");}
                else {m_ultimoValorExpr = vEsq / vDir;}
                break;
            case BinaryOp::Mod:
                if (vDir == 0) {throw RuntimeException("Erro de Execucao: Modulo por zero.");}
                else{m_ultimoValorExpr = vEsq % vDir;}
                break;
            case BinaryOp::Equal:
                m_ultimoValorExpr = (vEsq == vDir);
                break;
            case BinaryOp::NotEqual:
                m_ultimoValorExpr = (vEsq != vDir);
                break;
            case BinaryOp::LessThan:
                m_ultimoValorExpr = (vEsq < vDir);
                break;
            case BinaryOp::LessEqual:
                m_ultimoValorExpr = (vEsq <= vDir);
                break;
            case BinaryOp::GreaterThan:
                m_ultimoValorExpr = (vEsq > vDir);
                break;
            case BinaryOp::GreaterEqual:
                m_ultimoValorExpr = (vEsq >= vDir);
                break;
            default: 
                break;
        }
    }
    //Operações com double
    else if((std::holds_alternative<double>(esq) || std::holds_alternative<int>(esq))
    && (std::holds_alternative<double>(dir) || std::holds_alternative<int>(dir))) {
        double vEsq = std::holds_alternative<double>(esq) ? std::get<double>(esq) : std::get<int>(esq);
        double vDir = std::holds_alternative<double>(dir) ? std::get<double>(dir) : std::get<int>(dir);

        switch(node->op) {
            case BinaryOp::Add:
                m_ultimoValorExpr = vEsq + vDir;
                break;
            case BinaryOp::Sub:
                m_ultimoValorExpr = vEsq - vDir;
                break;
            case BinaryOp::Mul:
                m_ultimoValorExpr = vEsq * vDir;
                break;
            case BinaryOp::Div:
                if (vDir == 0.0 || vDir == -0.0) {throw RuntimeException("Erro de Execucao: Divisao por zero.");}
                m_ultimoValorExpr = vEsq / vDir;
                break;
            case BinaryOp::Equal:
                m_ultimoValorExpr = (vEsq == vDir);
                break;
            case BinaryOp::NotEqual:
                m_ultimoValorExpr = (vEsq != vDir);
                break;
            case BinaryOp::LessThan:
                m_ultimoValorExpr = (vEsq < vDir);
                break;
            case BinaryOp::LessEqual:
                m_ultimoValorExpr = (vEsq <= vDir);
                break;
            case BinaryOp::GreaterThan:
                m_ultimoValorExpr = (vEsq > vDir);
                break;
            case BinaryOp::GreaterEqual:
                m_ultimoValorExpr = (vEsq >= vDir);
                break;
            default:
                break;
        }
    }
    // Operações com caracteres
    else if (std::holds_alternative<char>(esq) && std::holds_alternative<char>(dir)){

        char cEsq = std::get<char>(esq);
        char cDir = std::get<char>(dir);

        switch (node->op) {
            case BinaryOp::Equal:
                m_ultimoValorExpr = (cEsq == cDir);
                break;

            case BinaryOp::NotEqual:
                m_ultimoValorExpr = (cEsq != cDir);
                break;

            case BinaryOp::LessThan:
                m_ultimoValorExpr = (cEsq < cDir);
                break;

            case BinaryOp::LessEqual:
                m_ultimoValorExpr = (cEsq <= cDir);
                break;

            case BinaryOp::GreaterThan:
                m_ultimoValorExpr = (cEsq > cDir);
                break;

            case BinaryOp::GreaterEqual:
                m_ultimoValorExpr = (cEsq >= cDir);
                break;

            default:
                throw RuntimeException(
                    "Erro de Execucao: Operacao invalida para caracteres."
                );
        }
    }
    //Operações lógicas booleanas
    else if (std::holds_alternative<bool>(esq) && std::holds_alternative<bool>(dir)) {
        bool bEsq = std::get<bool>(esq);
        bool bDir = std::get<bool>(dir);

        switch (node->op) {
            case BinaryOp::LogicalAnd:
                m_ultimoValorExpr = bEsq && bDir;
                break;
            case BinaryOp::LogicalOr:
                m_ultimoValorExpr = bEsq || bDir;
                break;
            case BinaryOp::Equal:
                m_ultimoValorExpr = (bEsq == bDir);
                break;
            case BinaryOp::NotEqual:
                m_ultimoValorExpr = (bEsq != bDir); 
                break;
            default:
                break;
        }
    }
    else if (std::holds_alternative<std::shared_ptr<ObjectValue>>(esq) && 
             std::holds_alternative<std::shared_ptr<ObjectValue>>(dir)) {
        auto pEsq = std::get<std::shared_ptr<ObjectValue>>(esq);
        auto pDir = std::get<std::shared_ptr<ObjectValue>>(dir);

        switch (node->op) {
            case BinaryOp::Equal:
                //Se ambos apontarem para o mesmo objeto ou se ambos forem nullptr, o resultado é true
                m_ultimoValorExpr = (pEsq == pDir);
                break;
            case BinaryOp::NotEqual:
                m_ultimoValorExpr = (pEsq != pDir);
                break;
            default:
                throw RuntimeException("Erro de Execucao: Operacao binaria invalida para ponteiros de objeto.");
        }
    } 
}

//Avalia negação lógica e sinal negativo
void Interpreter::visit(UnaryExprNode* node) {
    node->fator->accept(this);
    //Trata o operador de endereço '&' antes de avaliar o fator para não buscar o valor
    if (node->op == UnaryOp::AddressOf) {
        if (auto idNode = dynamic_cast<IdentifierNode*>(node->fator.get())) {
            //Em um interpretador baseado em árvore puro, simular o endereço físico de variáveis 
            //locais é feito obtendo o valor atual da variável e propagando-o.
            m_ultimoValorExpr = m_env.obter(idNode->nome);
            return;
        }
    }

    //Avaliação padrão dos outros operadores unários
    node->fator->accept(this);
    
    if(node->op == UnaryOp::LogicalNot && std::holds_alternative<bool>(m_ultimoValorExpr)) {
        m_ultimoValorExpr = !std::get<bool>(m_ultimoValorExpr);
    } else if(node->op == UnaryOp::Negate && std::holds_alternative<int>(m_ultimoValorExpr)) {
        m_ultimoValorExpr = -std::get<int>(m_ultimoValorExpr);
    } else if(node->op == UnaryOp::Negate && std::holds_alternative<double>(m_ultimoValorExpr)) {
        m_ultimoValorExpr = -std::get<double>(m_ultimoValorExpr);
    } else if(node->op == UnaryOp::Deref) {
        if (std::holds_alternative<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr)) {
            auto obj = std::get<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr);
            if (!obj) {
                throw RuntimeException("Erro de Execucao: Desreferenciacao de ponteiro nulo.");
            }
        }
    }
}

//Modifica com ++ ou --
void Interpreter::visit(IncDecExprNode* node) {
    Value v = m_env.obter(node->identificador);
    if(std::holds_alternative<int>(v)) {
        int val = std::get<int>(v);
        if(node->op == IncDecOp::Increment) val++;
        else val--;
        m_env.atribuir(node->identificador, val);
        m_ultimoValorExpr = val;
    }
}

//Chamada de função: aloca escopo local, passa parâmetros por valor e executa o corpo
void Interpreter::visit(FunctionCallExprNode* node) {
    auto it = m_funcoes.find(node->nomeFuncao);
    if(it == m_funcoes.end()) {
        throw RuntimeException("Erro de Execucao: Funcao nao declarada: " + node->nomeFuncao);
    }

    FunctionNode* func = it->second;
    std::vector<Value> valoresArgs;

    //Avalia os argumentos antes de mudar de escopo
    for(auto& arg : node->argumentos) {
        arg->accept(this);
        valoresArgs.push_back(m_ultimoValorExpr);
    }

    //Entra no escopo da nova função e atribui os parâmetros
    m_env.entrarEscopo();
    for(size_t i = 0; i < func->parametros.size(); ++i) {
        m_env.definir(func->parametros[i].nome, valoresArgs[i]);
    }

    //Executa a função
    func->accept(this);
    m_env.sairEscopo();
}
FunctionNode* Interpreter::buscarMetodo(const std::string& nomeClasse, const std::string& nomeMetodo) const {
    auto it = m_classes.find(nomeClasse);
    if (it == m_classes.end()) {
        return nullptr;
    }

    for (const auto& metodo : it->second->metodos) {
        if (metodo->nome == nomeMetodo) {
            return metodo.get();
        }
    }
    return nullptr;
}
//Acessa membros de objetos (. e ->)
void Interpreter::visit(MemberAccessExprNode* node) {
    node->objeto->accept(this);

    if(std::holds_alternative<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr)) {
        auto obj = std::get<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr);
        if(!obj) {
            throw RuntimeException("Erro de Execucao: Desreferenciacao de ponteiro nulo.");
        }
        if(obj->campos.find(node->nomeMembro) != obj->campos.end()) {
            m_ultimoValorExpr = obj->campos[node->nomeMembro];
        } else {
            FunctionNode* func = buscarMetodo(obj->nomeClasse, node->nomeMembro);
            if(func == nullptr) {
                throw RuntimeException("Erro de Execucao: metodo nao declarada: " + obj->nomeClasse);
            } else {
                std::vector<Value> valoresArgs;
                
                //Avalia os argumentos antes de mudar de escopo
                for(auto& arg : node->metodo->argumentos) {
                    arg->accept(this);
                    valoresArgs.push_back(m_ultimoValorExpr);
                }

                m_env.entrarEscopo();
                m_env.definir("this", obj);
                for(size_t i = 0; i < func->parametros.size(); ++i) {
                    m_env.definir(func->parametros[i].nome, valoresArgs[i]);
                }
                func->accept(this);
                m_env.sairEscopo();
            }
        }
    }
}

//Blocos de código entre chaves
void Interpreter::visit(BlockStmtNode* node) {
    m_env.entrarEscopo();
    for(const auto& cmd : node->comandos) {
        if(cmd) cmd->accept(this);
    }
    m_env.sairEscopo();
}

//Instancia objeto com campos zerados/padrão conforme sua definição
std::shared_ptr<ObjectValue> Interpreter::criarInstanciaObjeto(const std::string& nomeClasse) {
    auto obj = std::make_shared<ObjectValue>();
    obj->nomeClasse = nomeClasse;

    auto it = m_classes.find(nomeClasse);
    if(it != m_classes.end()) {
        for(const auto& campo : it->second->campos) {
            switch(campo.tipo.kind) {
                case DataTypeKind::Int:
                    obj->campos[campo.nome] = 0;
                    break;
                case DataTypeKind::Double:
                    obj->campos[campo.nome] = 0.0;
                    break;
                case DataTypeKind::Bool:
                    obj->campos[campo.nome] = false;
                    break;
                case DataTypeKind::Char:
                    obj->campos[campo.nome] = '\0';
                    break;
                case DataTypeKind::Class:
                    obj->campos[campo.nome] = criarInstanciaObjeto(campo.tipo.name);
                    break;
                default:
                    obj->campos[campo.nome] = 0;
                    break;
            }
        }
    }
    return obj;
}

//Declaração de variáveis com inicializador opcional
void Interpreter::visit(VariableDeclStmtNode* node) {
    for(const auto& decl : node->declaradores) {
        Value val = 0;

        if(node->tipoBasico.kind == DataTypeKind::Bool) val = false;
        else if(node->tipoBasico.kind == DataTypeKind::Double) val = 0.0;
        else if(node->tipoBasico.kind == DataTypeKind::Char) val = '\0';
        else if(node->tipoBasico.kind == DataTypeKind::Class) {
            val = criarInstanciaObjeto(node->tipoBasico.name);
        }

        if(decl.inicializador) {
            decl.inicializador->accept(this);
            val = m_ultimoValorExpr;
        }

        m_env.definir(decl.nome, val);
    }
}

//Atualização de variáveis
void Interpreter::visit(AssignmentStmtNode* node) {
     //Avalia o lado direito primeiro para descobrir o valor que será guardado
    node->direita->accept(this);
    Value valorDireito = m_ultimoValorExpr;

    //Resolve onde gravar com base no tipo do nó da esquerda
    if (auto idNode = dynamic_cast<IdentifierNode*>(node->esquerda.get())) {
        //Caso simples (Ex x = 10;)
        m_env.atribuir(idNode->nome, valorDireito);
    } 
    else if (auto memNode = dynamic_cast<MemberAccessExprNode*>(node->esquerda.get())) {
        //Caso de objeto (Ex: v.x = 10;)
        memNode->objeto->accept(this);
        if (std::holds_alternative<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr)) {
            auto obj = std::get<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr);
            if (!obj) throw RuntimeException("Erro de Execucao: Acesso a membro em ponteiro nulo.");
            obj->campos[memNode->nomeMembro] = valorDireito;
        }
    }
    else if (auto unNode = dynamic_cast<UnaryExprNode*>(node->esquerda.get())) {
        //Caso de ponteiro: (Ex: *p = 10;)
        if (unNode->op == UnaryOp::Deref) {
            unNode->fator->accept(this);
            if (auto subId = dynamic_cast<IdentifierNode*>(unNode->fator.get())) {
                m_env.atribuir(subId->nome, valorDireito);
            }
        }
    }
}

//Estrutura condicional if/else
void Interpreter::visit(IfStmtNode* node) {
    node->condicao->accept(this);
    if(std::get<bool>(m_ultimoValorExpr)) {
        if(node->comandoThen) node->comandoThen->accept(this);
    } else if(node->comandoElse) {
        node->comandoElse->accept(this);
    }
}

//Laço while com suporte a break e continue via exceção
void Interpreter::visit(WhileStmtNode* node) {
    while(true) {
        node->condicao->accept(this);
        if(!std::get<bool>(m_ultimoValorExpr)) break;

        try {
            if(node->comandoBody) node->comandoBody->accept(this);
        } catch(const BreakException&) {
            break;
        } catch(const ContinueException&) {
            continue;
        }
    }
}

void Interpreter::visit(BreakStmtNode* node) { 
    throw BreakException(); 
}

void Interpreter::visit(ContinueStmtNode* node) {
    throw ContinueException();
}

void Interpreter::visit(ReturnStmtNode* node) {
    Value retVal = 0;
    if(node->expressao) {
        node->expressao->accept(this);
        retVal = m_ultimoValorExpr;
    }
    throw ReturnException{retVal};
}

void Interpreter::visit(ExpressionStmtNode* node) {
    if(node->expressao) {
        node->expressao->accept(this);
    }
}