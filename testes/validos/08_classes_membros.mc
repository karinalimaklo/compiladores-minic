struct Vetor {
    int x;
    int y;
};

class Pessoa {
    int idade;
    Vetor* posicao;
};

int main() {
    Vetor v;
    v.x = 10;
    v.y = 20;

    Pessoa p;
    p.idade = 25;
    
    Pessoa* ptrPessoa = &p;
    ptrPessoa->idade = 26;
    
    return 0;
}
