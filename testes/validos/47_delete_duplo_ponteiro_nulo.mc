class Pessoa {
    int idade;
};

int main() {
    Pessoa* p = new Pessoa{20};

    delete p;
    delete p;

    return 0;
}