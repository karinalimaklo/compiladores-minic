int main() {
    int x = 0;
    int y = 0;
    int* p = &x;
    *p = 1;
    p = &y;
    *p = 2;
    if (x != 1) {
        int falhou = 1 / 0;
    }
    if (y != 2) {
        int falhou = 1 / 0;
    }
    return 0;
}
