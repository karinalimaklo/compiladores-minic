// main.cpp — lexer do MiniC++
//
// Pipeline: tabela de regex -> AST (parserRegex) -> AFN geral (Thompson)
//           -> AFD (subconjuntos) -> varredura do programa com maximal munch.
//
// Uso:
//   ./lexer programa.mcpp      le o programa de um arquivo
//   ./lexer                    le o programa da entrada padrao (Ctrl+D para terminar)

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "afd.hpp"
#include "afdConstructor.hpp"
#include "afn.hpp"
#include "afnConstructor.hpp"
#include "gramaticaCod.hpp"
#include "token.hpp"

// ---------------------------------------------------------------------------
// Nome legivel de cada tipo de token (so para imprimir)
// ---------------------------------------------------------------------------
const char* nomeToken(TipoToken t) {
    switch (t) {
        case TipoToken::IntLit:       return "IntLit";
        case TipoToken::DoubleLit:    return "DoubleLit";
        case TipoToken::CharLit:      return "CharLit";
        case TipoToken::StringLit:    return "StringLit";
        case TipoToken::Ident:        return "Ident";
        case TipoToken::PalInt:       return "PalInt";
        case TipoToken::PalChar:      return "PalChar";
        case TipoToken::PalDouble:    return "PalDouble";
        case TipoToken::PalBool:      return "PalBool";
        case TipoToken::PalString:    return "PalString";
        case TipoToken::PalVoid:      return "PalVoid";
        case TipoToken::PalTrue:      return "PalTrue";
        case TipoToken::PalFalse:     return "PalFalse";
        case TipoToken::PalIf:        return "PalIf";
        case TipoToken::PalElse:      return "PalElse";
        case TipoToken::PalWhile:     return "PalWhile";
        case TipoToken::PalFor:       return "PalFor";
        case TipoToken::PalBreak:     return "PalBreak";
        case TipoToken::PalContinue:  return "PalContinue";
        case TipoToken::PalRet:       return "PalRet";
        case TipoToken::PalCin:       return "PalCin";
        case TipoToken::PalCout:      return "PalCout";
        case TipoToken::Plus:         return "Plus";
        case TipoToken::Min:          return "Min";
        case TipoToken::Star:         return "Star";
        case TipoToken::Div:          return "Div";
        case TipoToken::Percent:      return "Percent";
        case TipoToken::Assign:       return "Assign";
        case TipoToken::PlusAssign:   return "PlusAssign";
        case TipoToken::MinAssign:    return "MinAssign";
        case TipoToken::Eq:           return "Eq";
        case TipoToken::NotEq:        return "NotEq";
        case TipoToken::Gt:           return "Gt";
        case TipoToken::Ge:           return "Ge";
        case TipoToken::Lt:           return "Lt";
        case TipoToken::Le:           return "Le";
        case TipoToken::AndAnd:       return "AndAnd";
        case TipoToken::OrOr:         return "OrOr";
        case TipoToken::Not:          return "Not";
        case TipoToken::PlusPLus:     return "PlusPlus";
        case TipoToken::MinMin:       return "MinMin";
        case TipoToken::Shl:          return "Shl";
        case TipoToken::Shr:          return "Shr";
        case TipoToken::Interrogacao: return "Interrogacao";
        case TipoToken::DoisPontos:   return "DoisPontos";
        case TipoToken::ColEsquerda:  return "ColEsquerda";
        case TipoToken::ColDireita:   return "ColDireita";
        case TipoToken::ChavesEsq:    return "ChavesEsq";
        case TipoToken::ChavesDir:    return "ChavesDir";
        case TipoToken::ColchEsq:     return "ColchEsq";
        case TipoToken::ColchDir:     return "ColchDir";
        case TipoToken::Virg:         return "Virg";
        case TipoToken::PontoVirg:    return "PontoVirg";
        case TipoToken::FimArquivo:   return "FimArquivo";
    }
    return "?";
}

// ---------------------------------------------------------------------------
// Erro lexico: guarda a posicao e o trecho que nao foi reconhecido
// ---------------------------------------------------------------------------
struct ErroLexico {
    Posicao posicao;
    std::string trecho;
};

// Atualiza linha/coluna depois de consumir o caractere c
void avancarPosicao(Posicao& p, char c) {
    if (c == '\n') {
        p.linha++;
        p.coluna = 1;
    } else {
        p.coluna++;
    }
}

bool ehEspaco(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// ---------------------------------------------------------------------------
// Tokenizacao com maximal munch
//
// A partir de cada posicao, anda no AFD o maximo possivel e lembra o ULTIMO
// ponto em que passou por um estado de aceitacao. Quando o AFD trava
// (proximo == -1) ou a entrada acaba, o token e o trecho ate esse ponto.
// O tipo vem da etiqueta do estado (indice na tabela). Se o token for um
// Ident, consulta a tabela de palavras-chave.
// ---------------------------------------------------------------------------
std::vector<Token> tokenizar(const std::string& fonte,
                             const AFD& afd,
                             const std::vector<EntradaLexica>& tabela,
                             std::vector<ErroLexico>& erros) {
    std::vector<Token> tokens;
    const auto& chaves = palavrasChave();

    std::size_t pos = 0;
    Posicao p;  // linha 1, coluna 1

    while (pos < fonte.size()) {
        // 1. pular espacos em branco
        if (ehEspaco(fonte[pos])) {
            avancarPosicao(p, fonte[pos]);
            pos++;
            continue;
        }

        // 2. andar no AFD o maximo possivel
        int q = afd.getInicial();
        std::size_t i = pos;
        std::size_t fimAceito = pos;   // fim (exclusivo) do maior prefixo aceito
        int etiquetaAceita = -1;

        while (i < fonte.size()) {
            q = afd.proximo(q, fonte[i]);
            if (q < 0) break;          // estado morto: nao da para continuar
            i++;
            int e = afd.getEstado(q).etiqueta;
            if (e >= 0) {              // estado de aceitacao: lembra este ponto
                fimAceito = i;
                etiquetaAceita = e;
            }
        }

        // 3. nenhum prefixo aceito: erro lexico em um caractere
        if (etiquetaAceita < 0) {
            erros.push_back({p, std::string(1, fonte[pos])});
            avancarPosicao(p, fonte[pos]);
            pos++;
            continue;
        }

        // 4. monta o token com o maior prefixo aceito
        Token tok;
        tok.lexeme = fonte.substr(pos, fimAceito - pos);
        tok.tipo = tabela[etiquetaAceita].tipo;
        tok.posicao = p;

        if (tok.tipo == TipoToken::Ident) {
            auto it = chaves.find(tok.lexeme);
            if (it != chaves.end()) tok.tipo = it->second;
        }
        tokens.push_back(tok);

        for (std::size_t k = pos; k < fimAceito; k++) avancarPosicao(p, fonte[k]);
        pos = fimAceito;
    }

    Token fim;
    fim.tipo = TipoToken::FimArquivo;
    fim.lexeme = "";
    fim.posicao = p;
    tokens.push_back(fim);
    return tokens;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // 1. ler o programa de entrada
    std::string fonte;
    if (argc >= 2) {
        std::ifstream arq(argv[1]);
        if (!arq) {
            std::cerr << "Nao foi possivel abrir o arquivo: " << argv[1] << "\n";
            return 1;
        }
        std::stringstream ss;
        ss << arq.rdbuf();
        fonte = ss.str();
    } else {
        std::stringstream ss;
        ss << std::cin.rdbuf();
        fonte = ss.str();
    }

    // 2. tabela de tokens (regex -> AST)
    std::vector<EntradaLexica> tabela = criarTabelaTokens();

    // 3. AFN geral (Thompson, um fragmento por token, etiqueta = indice)
    ConstrutorAFN construtorAfn;
    AFN afn = construtorAfn.gerarAfnFinal(tabela);

    // 4. AFD (construcao de subconjuntos)
    ConstrutorAFD construtorAfd;
    AFD afd = construtorAfd.construir(afn);

    std::cerr << "AFN: " << afn.quantidadeEstados() << " estados | "
              << "AFD: " << afd.quantidadeEstados() << " estados\n\n";

    // 5. tokenizar
    std::vector<ErroLexico> erros;
    std::vector<Token> tokens = tokenizar(fonte, afd, tabela, erros);

    // 6. imprimir os tokens
    for (const Token& t : tokens) {
        std::cout << t.posicao.linha << ":" << t.posicao.coluna << "\t"
                  << nomeToken(t.tipo);
        if (!t.lexeme.empty()) std::cout << "\t'" << t.lexeme << "'";
        std::cout << "\n";
    }

    // 7. imprimir os erros
    for (const ErroLexico& e : erros) {
        std::cerr << "Erro lexico em " << e.posicao.linha << ":" << e.posicao.coluna
                  << ": caractere inesperado '" << e.trecho << "'\n";
    }

    return erros.empty() ? 0 : 2;
}