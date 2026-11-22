class Pessoa {
    int idade;
};

int main() {
    Pessoa* p = new Pessoa{20};
    Pessoa* q = p;

    delete p;

    q->idade = 30;

    return 0;
}