int main() {
    int x = 1;
    int y = 1;
    int* p = &x;
    int* q = &x;
    int* r = &y;
    if (p != q) {
        int falhou = 1 / 0;
    }
    if (p == r) {
        int falhou = 1 / 0;
    }
    if (p == nullptr) {
        int falhou = 1 / 0;
    }
    int* n = nullptr;
    if (n != nullptr) {
        int falhou = 1 / 0;
    }
    return 0;
}
