int main() {
    int x = 1;
    int* p = &x;
    x = 5;
    if (*p != 5) {
        int falhou = 1 / 0;
    }
    return 0;
}
