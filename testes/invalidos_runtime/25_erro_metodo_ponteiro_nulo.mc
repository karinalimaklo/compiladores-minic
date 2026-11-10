struct P {
    int x;
    int obter() {
        return this->x;
    }
};

int main() {
    P* q = nullptr;
    int r = q->obter();
    return 0;
}
