struct Contador {
    int valor;

    void definir(int v) {
        this->valor = v;
    }

    int obter() {
        return this->valor;
    }

    void incrementar() {
        this->valor = this->valor + 1;
    }
};

int main() {
    Contador c;
    c.definir(9);
    c.incrementar();
    c.incrementar();
    int r = c.obter();

    if (r != 11) {
        int falhou = 1 / 0;
    }
    if (c.valor != 11) {
        int falhou = 1 / 0;
    }
    return 0;
}
