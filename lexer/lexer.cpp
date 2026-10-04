#include "lexer.hpp"
#include "afdConstructor.hpp"
#include "afn.hpp"
#include "afnConstructor.hpp"

namespace {

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

}  // namespace

Lexer::Lexer() {
    tabela = criarTabelaTokens();

    ConstrutorAFN construtorAfn;
    AFN afn = construtorAfn.gerarAfnFinal(tabela);
    nEstadosAfn = afn.quantidadeEstados();

    ConstrutorAFD construtorAfd;
    afd = construtorAfd.construir(afn);
}

// Maximal munch: a partir de cada posicao, anda no AFD o maximo possivel e
// guarda o ULTIMO ponto em que passou por um estado de aceitacao. O token e o
// trecho ate esse ponto; o tipo vem da etiqueta (indice na tabela). Um Ident
// e reclassificado como palavra-chave se estiver na tabela de palavras-chave.
std::vector<Token> Lexer::tokenizar(const std::string& fonte,
                                    std::vector<ErroLexico>& erros) const {
    std::vector<Token> tokens;
    const auto& chaves = palavrasChave();

    std::size_t pos = 0;
    Posicao p;

    while (pos < fonte.size()) {
        if (ehEspaco(fonte[pos])) {
            avancarPosicao(p, fonte[pos]);
            pos++;
            continue;
        }

        int q = afd.getInicial();
        std::size_t i = pos;
        std::size_t fimAceito = pos;
        int etiquetaAceita = -1;

        while (i < fonte.size()) {
            q = afd.proximo(q, fonte[i]);
            if (q < 0) break;
            i++;
            int e = afd.getEstado(q).etiqueta;
            if (e >= 0) {
                fimAceito = i;
                etiquetaAceita = e;
            }
        }

        if (etiquetaAceita < 0) {
            erros.push_back({p, std::string(1, fonte[pos])});
            avancarPosicao(p, fonte[pos]);
            pos++;
            continue;
        }

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

    tokens.push_back(Token{TipoToken::FimArquivo, "", p});
    return tokens;
}

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
