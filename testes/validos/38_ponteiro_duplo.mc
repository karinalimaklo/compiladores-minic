int main() {
    int x = 1;
    int* p = &x;
    int** pp = &p;
    **pp = 7;
    if (x != 7) {
        int falhou = 1 / 0;
    }
    if (*pp != p) {
        int falhou = 1 / 0;
    }
    int y = 3;
    *pp = &y;
    if (*p != 3) {
        int falhou = 1 / 0;
    }
    return 0;
}
