void incrementar(int* p) {
    *p = *p + 1;
}

void trocar(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

int main() {
    int x = 1;
    incrementar(&x);
    incrementar(&x);
    if (x != 3) {
        int falhou = 1 / 0;
    }
    int a = 10;
    int b = 20;
    trocar(&a, &b);
    if (a != 20) {
        int falhou = 1 / 0;
    }
    if (b != 10) {
        int falhou = 1 / 0;
    }
    return 0;
}
