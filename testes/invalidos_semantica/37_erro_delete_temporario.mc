class Pessoa {
    int idade;
};

int main() {
    delete new Pessoa{20};

    return 0;
}