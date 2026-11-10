struct Ponto {
    int x;

    int meuX(Ponto outro) {
        return this->x;
    }

    int xDoOutro(Ponto outro) {
        return outro.x;
    }

    void mover(int dx) {
        this->x = this->x + dx;
    }
};

int main() {
    Ponto p;
    Ponto q;
    p.x = 1;
    q.x = 42;

    if (p.meuX(q) != 1) {
        int falhou = 1 / 0;
    }
    if (p.xDoOutro(q) != 42) {
        int falhou = 1 / 0;
    }

    Ponto* ptr = &q;
    ptr->mover(8);
    if (q.x != 50) {
        int falhou = 1 / 0;
    }
    if (p.x != 1) {
        int falhou = 1 / 0;
    }
    return 0;
}
