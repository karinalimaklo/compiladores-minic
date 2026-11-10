struct P {
    int x;
    int f(int a) {
        return a;
    }
};

int main() {
    P p;
    int r = p.f(true);
    return 0;
}
