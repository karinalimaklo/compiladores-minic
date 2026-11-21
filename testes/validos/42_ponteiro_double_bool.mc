int main() {
    double d = 1.5;
    double* pd = &d;
    *pd = 2.5;
    if (d != 2.5) {
        int falhou = 1 / 0;
    }
    bool b = false;
    bool* pb = &b;
    *pb = true;
    if (!b) {
        int falhou = 1 / 0;
    }
    return 0;
}
