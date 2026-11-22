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
                m_ultimoValorExpr = std::shared_ptr<Celula>{};
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
    else if (std::holds_alternative<std::shared_ptr<Celula>>(esq) && 
             std::holds_alternative<std::shared_ptr<Celula>>(dir)) {
        auto pEsq = std::get<std::shared_ptr<Celula>> (esq);
        auto pDir = std::get<std::shared_ptr<Celula>> (dir);
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
    } else{
        throw RuntimeException("Operação binária inválida!");
    }
}

// Resolve um LValue e retorna o endereço do Value que deverá ser modificado.
std::shared_ptr<Celula> Interpreter::resolverLValue(ExpressionNode* node) {
    if (!node) {
        throw RuntimeException("Erro de Execucao: expressao nula ao resolver LValue.");
    }
    // Caso 1: identificador simples (Ex: x)
    if (auto idNode = dynamic_cast<IdentifierNode*>(node)) {
        return m_env.resolver(idNode->nome);
    }

    // Caso 2: acesso a campo (objeto.campo ou ponteiro->campo)
    if (auto memNode = dynamic_cast<MemberAccessExprNode*>(node)) {
        memNode->objeto->accept(this);

        std::shared_ptr<ObjectValue> obj;

        if (memNode->isArrow) {
            // O operador -> aceita um ponteiro para objeto e ObjectValue diretamente, como acontece com this.
            if (std::holds_alternative<std::shared_ptr<Celula>>(m_ultimoValorExpr)) {

                auto celula = std::get<std::shared_ptr<Celula>>(m_ultimoValorExpr);

                if (!celula) {
                    throw RuntimeException("Erro de Execucao: acesso por ponteiro nulo.");
                }
                if (celula->liberada) {
                    throw RuntimeException("Erro de Execucao: tentativa de acessar memoria ja desalocada.");
                }
                if (!std::holds_alternative<std::shared_ptr<ObjectValue>>(celula->valor)) {
                    throw RuntimeException("Erro de Execucao: ponteiro nao aponta para objeto.");
                }

                obj = std::get<std::shared_ptr<ObjectValue>>(celula->valor);

            } else if (std::holds_alternative<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr)) {

                obj = std::get<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr);

            } else {
                throw RuntimeException("Erro de Execucao: operador '->' exige um ponteiro.");
            }

        } else {
            // O operador . exige um objeto
            if (!std::holds_alternative<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr)) {
                throw RuntimeException("Erro de Execucao: acesso a campo exige um objeto.");
            }

            obj = std::get<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr);
        }

        if (!obj) {
            throw RuntimeException("Erro de Execucao: acesso a objeto nulo.");
        }

        auto campo = obj->campos.find(memNode->nomeMembro);

        if (campo == obj->campos.end()) {
            throw RuntimeException("Erro de Execucao: campo '" + memNode->nomeMembro + "' nao encontrado.");
        }

        return campo->second;
    }

    // Caso 3: desreferenciação (*p)
    if (auto unNode = dynamic_cast<UnaryExprNode*>(node)) {
        if (unNode->op == UnaryOp::Deref) {
            unNode->fator->accept(this);

            if (!std::holds_alternative<std::shared_ptr<Celula>>(m_ultimoValorExpr)) {
                throw RuntimeException("Erro de Execucao: operando de '*' nao e um ponteiro.");
            }

            auto celula = std::get<std::shared_ptr<Celula>>(m_ultimoValorExpr);

            if (!celula) {
                throw RuntimeException("Erro de Execucao: tentativa de acessar ponteiro nulo.");
            }

            if (celula->liberada) {
                throw RuntimeException("Erro de Execucao: tentativa de acessar memoria ja desalocada.");
            }

            return celula;
        }
    }

    throw RuntimeException("Erro de Execucao: expressao nao representa um LValue valido.");
}

//Avalia negação lógica e sinal negativo
void Interpreter::visit(UnaryExprNode* node) {
    //Trata o operador de endereço '&' antes de avaliar o fator para não buscar o valor
    if (node->op == UnaryOp::AddressOf) {
        auto celula = resolverLValue(node->fator.get());
        m_ultimoValorExpr = celula;
        return;
    } else if(node->op == UnaryOp::Deref) {
        node->fator->accept(this);
        if (!std::holds_alternative<std::shared_ptr<Celula>>(m_ultimoValorExpr)) {
            throw RuntimeException("Erro de Execucao: operando de '*' nao e um ponteiro.");
        }
        auto celula = std::get<std::shared_ptr<Celula>>(m_ultimoValorExpr);
        if (!celula) {
            throw RuntimeException("Erro de Execucao: desreferenciacao de ponteiro nulo.");
        }
        if (celula->liberada) {
            throw RuntimeException("Erro de Execucao: tentativa de acessar memoria ja desalocada.");
        }
        m_ultimoValorExpr = celula->valor;
        return;
    }

    //Avaliação padrão dos outros operadores unários
    node->fator->accept(this);
    
    if(node->op == UnaryOp::LogicalNot && std::holds_alternative<bool>(m_ultimoValorExpr)) {
        m_ultimoValorExpr = !std::get<bool>(m_ultimoValorExpr);
    } else if(node->op == UnaryOp::Negate && std::holds_alternative<int>(m_ultimoValorExpr)) {
        m_ultimoValorExpr = -std::get<int>(m_ultimoValorExpr);
    } else if(node->op == UnaryOp::Negate && std::holds_alternative<double>(m_ultimoValorExpr)) {
        m_ultimoValorExpr = -std::get<double>(m_ultimoValorExpr);
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
void Interpreter:: verificaCelula(MemberAccessExprNode* node) {
    node->objeto->accept(this);
    if(std::holds_alternative<std::shared_ptr<Celula>>(m_ultimoValorExpr)) {
        auto obj = std::get<std::shared_ptr<Celula>>(m_ultimoValorExpr);
        if(!obj) {
            throw RuntimeException("Erro de Execucao: Desreferenciacao de ponteiro nulo.");
        }
        if (obj->liberada) {
            throw RuntimeException("Erro de Execucao: tentativa de acessar memoria ja desalocada.");
        }
        if (!std::holds_alternative<std::shared_ptr<ObjectValue>>(obj->valor)) {
            throw RuntimeException("Erro de Execucao: ponteiro nao aponta para objeto.");
        }
        m_ultimoValorExpr = obj->valor;    
    } else if(!std::holds_alternative<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr)) {
        throw RuntimeException("Erro de Execucao: acesso com '->' exige um objeto ou ponteiro.");
    } 
}
//Acessa membros de objetos (. e ->)
void Interpreter::visit(MemberAccessExprNode* node) {
    std::shared_ptr<ObjectValue> obj;
    if(node->isArrow) {
        verificaCelula(node);
    } else {
        node->objeto->accept(this);
    }
    if (!std::holds_alternative<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr)) {
        throw RuntimeException("Erro de Execucao: acesso a membro exige um objeto.");
    }

    obj = std::get<std::shared_ptr<ObjectValue>>(m_ultimoValorExpr);

    if (!obj) {
        throw RuntimeException("Erro de Execucao: acesso a objeto nulo.");
    }

    auto campo = obj->campos.find(node->nomeMembro);

    if (campo != obj->campos.end()) {
        if (!campo->second) {
            throw RuntimeException("Erro de Execucao: campo sem celula de armazenamento.");
        }
        m_ultimoValorExpr = campo->second->valor;
        return;
    }

    // Se não encontrou um campo, procura um método
    FunctionNode* func = buscarMetodo(obj->nomeClasse, node->nomeMembro);

    if (func == nullptr) {
        throw RuntimeException("Erro de Execucao: metodo nao declarado: " + node->nomeMembro);
    }

    std::vector<Value> valoresArgs;

    if (node->metodo) {
        for (auto& arg : node->metodo->argumentos) {
            arg->accept(this);
            valoresArgs.push_back(m_ultimoValorExpr);
        }
    }

    m_env.entrarEscopo();

    try {
        m_env.definir("this", obj);

        for (size_t i = 0; i < func->parametros.size(); ++i) {
            m_env.definir(func->parametros[i].nome, valoresArgs[i]);
        }

        func->accept(this);
        m_env.sairEscopo();

    } catch (...) {
        m_env.sairEscopo();
        throw;
    }
}

//Blocos de código entre chaves
void Interpreter::visit(BlockStmtNode* node) {
    struct GuardaEscopo {
        Environment& env;
        ~GuardaEscopo() { env.sairEscopo(); }
    };
    m_env.entrarEscopo();
    GuardaEscopo guarda{m_env};

    for(const auto& cmd : node->comandos) {
        if(cmd) cmd->accept(this);
    }
}

//Instancia objeto com campos zerados/padrão conforme sua definição
std::shared_ptr<ObjectValue> Interpreter::criarInstanciaObjeto(const std::string& nomeClasse) {
    auto obj = std::make_shared<ObjectValue>();
    obj->nomeClasse = nomeClasse;

    auto it = m_classes.find(nomeClasse);
    if(it != m_classes.end()) {
        for(const auto& campo : it->second->campos) {
            Value valor;
            if(campo.tipo.pointerLevel > 0) {
                valor = std::shared_ptr<Celula>{};
            } else {
            switch(campo.tipo.kind) {
                case DataTypeKind::Int:
                    valor = 0;
                    break;
                case DataTypeKind::Double:
                    valor = 0.0;
                    break;
                case DataTypeKind::Bool:
                    valor = false;
                    break;
                case DataTypeKind::Char:
                    valor = '\0';
                    break;
                case DataTypeKind::Class:
                    valor = criarInstanciaObjeto(campo.tipo.name);
                    break;
                default:
                    valor = 0;
                    break;
            }              
            }
            obj->campos[campo.nome] = std::make_shared<Celula>(Celula{valor});
        }
    }
    return obj;
}

//Declaração de variáveis com inicializador opcional
void Interpreter::visit(VariableDeclStmtNode* node) {
        for(const auto& decl : node->declaradores) {
            Value val;
            if(node->tipoBasico.pointerLevel > 0) {
                std::shared_ptr<Celula> cel_var {};
                val = cel_var;
            } else {
                if(node->tipoBasico.kind == DataTypeKind::Bool) val = false;
                else if(node->tipoBasico.kind == DataTypeKind::Double) val = 0.0;
                else if(node->tipoBasico.kind == DataTypeKind::Char) val = '\0';
                else if(node->tipoBasico.kind == DataTypeKind::Class) {
                    val = criarInstanciaObjeto(node->tipoBasico.name);
                }
            }
            if(decl.inicializador) {
                decl.inicializador->accept(this);
                val = m_ultimoValorExpr;
            }

            m_env.definir(decl.nome, val);
        }
}
    
//Atualização de variáveis e atributos
void Interpreter::visit(AssignmentStmtNode* node) {

    //Avalia primeiro o lado direito
    node->direita->accept(this);

    //Guarda o valor calculado
    Value valorDireito = m_ultimoValorExpr;

    // Obtém a célula real do lado esquerdo.
    auto destino = resolverLValue(node->esquerda.get());

    if (!destino) {
        throw RuntimeException("Erro de Execucao: destino de atribuicao invalido.");
    }
    // Atualiza o conteúdo da célula
    destino->valor = valorDireito;
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

void Interpreter::visit(NewExprNode* node) {
    auto cel_var = std::make_shared<Celula>();
    std::shared_ptr<ObjectValue> val_temp = nullptr;
    auto it = m_classes.find(node->tipo.name);

    if (node->tipo.pointerLevel == 0) {
        val_temp = criarInstanciaObjeto(node->tipo.name);

        if (!node->parametros.empty()) {
            if (it == m_classes.end()) {
                throw RuntimeException("Erro de Execucao: classe nao encontrada.");
            }

            if (node->parametros.size() != it->second->campos.size()) {
                throw RuntimeException("Erro de Execucao: quantidade de inicializadores invalida.");
            }

            size_t i = 0;

            for (const auto& campo : it->second->campos) {
                node->parametros[i]->accept(this);
                auto campoObj = val_temp->campos.find(campo.nome);

                if (campoObj == val_temp->campos.end() || !campoObj->second) {
                    throw RuntimeException("Erro de Execucao: campo sem armazenamento.");
                }

                campoObj->second->valor = m_ultimoValorExpr;
                ++i;
            }
        }

        cel_var->valor = val_temp;

    } else {
        if (!node->parametros.empty()) {
            node->parametros[0]->accept(this);
            cel_var->valor = m_ultimoValorExpr;
        } else {
            cel_var->valor = std::shared_ptr<Celula>{};
        }
    }

    cel_var->alocadaComNew = true; //Esta celula representa uma alocacao feita com new
    m_ultimoValorExpr = cel_var;
}

void Interpreter::visit(DeleteStmtNode* node) {

    if (!node || !node->expressao) {
        throw RuntimeException("Erro de Execucao: expressao invalida no comando 'delete'.");
    }

    //Caso especial: delete nullptr; nao precisa fazer nada.
    if (auto literal = dynamic_cast<LiteralNode*>(node->expressao.get())) {

        if (literal->valor == "nullptr") {
            return;
        }
    }

    //Localiza a celula que armazena o ponteiro.
    std::shared_ptr<Celula> celulaPonteiro =
        resolverLValue(node->expressao.get());

    if (!celulaPonteiro) {
        throw RuntimeException("Erro de Execucao: ponteiro invalido no comando 'delete'.");
    }

    //O valor armazenado precisa ser um ponteiro.
    if (!std::holds_alternative<std::shared_ptr<Celula>>(celulaPonteiro->valor)) {
        throw RuntimeException("Erro de Execucao: o operando de 'delete' nao e um ponteiro.");
    }

    std::shared_ptr<Celula> alvo = std::get<std::shared_ptr<Celula>>(celulaPonteiro->valor);

    //Delete de um ponteiro nulo é uma operacao segura
    if (!alvo) {
        return;
    }

    //Impede liberar uma variavel ou atributo que nao veio de new
    if (!alvo->alocadaComNew) {
        throw RuntimeException("Erro de Execucao: tentativa de liberar memoria que nao foi alocada com 'new'.");
    }

    //Impede uma segunda liberacao por meio de outro ponteiro que seja alias
    if (alvo->liberada) {
        throw RuntimeException("Erro de Execucao: tentativa de liberar memoria que ja foi desalocada.");
    }

    //Marca a celula como liberada
    alvo->liberada = true;

    //Limpa o objeto ou ponteiro armazenado na celula liberada.
    if (std::holds_alternative<std::shared_ptr<ObjectValue>>(alvo->valor)) {
        alvo->valor = std::shared_ptr<ObjectValue>{};
    } else if (std::holds_alternative<std::shared_ptr<Celula>>(alvo->valor)) {
        alvo->valor = std::shared_ptr<Celula>{};
    } else {
        throw RuntimeException("Erro de Execucao: conteudo invalido na memoria alocada.");
    }

    //O ponteiro utilizado no delete passa a ser nullptr
    celulaPonteiro->valor = std::shared_ptr<Celula>{};
}