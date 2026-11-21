int main() {
    int x = 42;
    int* p = &x;
    *p = 10;
    if (x != 10) {
        int falhou = 1 / 0;
    }
    return 0;
}
