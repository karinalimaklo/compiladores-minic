struct Ponto {
    int x;
};

int main() {
    Ponto p;
    p.x = 1;
    int* r = &p.x;
    *r = 8;
    if (p.x != 8) {
        int falhou = 1 / 0;
    }
    return 0;
}
