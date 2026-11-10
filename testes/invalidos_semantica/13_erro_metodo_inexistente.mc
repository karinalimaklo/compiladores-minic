struct P {
    int x;
    int f() {
        return 1;
    }
};

int main() {
    P p;
    int r = p.g();
    return 0;
}
