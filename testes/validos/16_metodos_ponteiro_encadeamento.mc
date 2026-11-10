struct Interno {
    int y;
};

class Externo {
    int x;
    Interno dentro;

    Externo* eu() {
        return this;
    }

    int obterX() {
        return this->x;
    }

    Interno pegarInterno() {
        return this->dentro;
    }
};

int usaPonteiro(Externo* e) {
    return e->obterX();
}

int main() {
    Externo e;
    e.x = 6;
    e.dentro.y = 4;

    if (e.eu()->obterX() != 6) {
        int falhou = 1 / 0;
    }
    if (e.pegarInterno().y != 4) {
        int falhou = 1 / 0;
    }
    if (usaPonteiro(&e) != 6) {
        int falhou = 1 / 0;
    }

    Externo* p = &e;
    if (p->obterX() != 6) {
        int falhou = 1 / 0;
    }
    return 0;
}
