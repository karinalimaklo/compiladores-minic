struct Ponto {
    int x;
};

int main() {
    Ponto p;
    p.x = 3;
    Ponto* q = &p;
    Ponto c = *q;
    if (c.x != 3) {
        int falhou = 1 / 0;
    }
    q->x = 9;
    if (p.x != 9) {
        int falhou = 1 / 0;
    }
    return 0;
}
