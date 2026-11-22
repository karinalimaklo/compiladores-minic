class Pessoa {
    int idade;
};

int main() {
    Pessoa* p = new Pessoa{20};
    Pessoa* q = p;

    delete p;
    delete q;

    return 0;
}