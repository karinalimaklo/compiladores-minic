struct P {
    int x;
    int f() {
        return 1;
    }
};

int main() {
    P p;
    p.f() = 3;
    return 0;
}
