struct No {
    int v;
    No* prox;
};

int main() {
    No* a = new No{1, nullptr};
    a->prox = new No{5, nullptr};
    a->prox->v = 7;
    if (a->prox->v != 7) {
        int falhou = 1 / 0;
    }
    No* b = a;
    b->v = 9;
    if (a->v != 9) {
        int falhou = 1 / 0;
    }
    return 0;
}