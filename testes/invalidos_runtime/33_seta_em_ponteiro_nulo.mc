struct No {
    int v;
    No* prox;
};

int main() {
    No* a = new No{1, nullptr};
    int x = a->prox->v;
    return 0;
}