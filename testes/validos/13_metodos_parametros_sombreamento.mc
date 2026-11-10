struct Calc {
    int base;

    int somar(int a, int b) {
        return a + b + this->base;
    }

    int identidade(int a) {
        return a;
    }
};

int main() {
    int a = 100;
    Calc c;
    c.base = 1;

    int r1 = c.somar(3, 4);
    if (r1 != 8) {
        int falhou = 1 / 0;
    }

    int r2 = c.identidade(5);
    if (r2 != 5) {
        int falhou = 1 / 0;
    }

    return 0;
}
