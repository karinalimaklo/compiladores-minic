struct P {
    int x;
    void f(P* outro) {
        this = outro;
    }
};

int main() {
    return 0;
}
