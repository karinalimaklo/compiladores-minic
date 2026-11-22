class Pessoa {
    int idade;
};

int main() {
    Pessoa* p = new Pessoa{20};

    delete p;

    p = new Pessoa{30};

    delete p;

    return 0;
}