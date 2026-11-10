struct Mat {
    int x;

    int fat(int n) {
        if (n <= 1) {
            return 1;
        }
        return n * this->fat(n - 1);
    }

    int dobro(int n) {
        return n * 2;
    }

    int quadruplo(int n) {
        return this->dobro(this->dobro(n));
    }
};

int main() {
    Mat m;
    if (m.fat(5) != 120) {
        int falhou = 1 / 0;
    }
    if (m.quadruplo(3) != 12) {
        int falhou = 1 / 0;
    }
    return 0;
}
