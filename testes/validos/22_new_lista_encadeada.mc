struct No {
    int v;
    No* prox;
};

int main() {
    No* lista = new No{1, new No{2, new No{3, nullptr}}};
    int soma = 0;
    No* atual = lista;
    while (atual != nullptr) {
        soma = soma + atual->v;
        atual = atual->prox;
    }
    if (soma != 6) {
        int falhou = 1 / 0;
    }
    return 0;
}