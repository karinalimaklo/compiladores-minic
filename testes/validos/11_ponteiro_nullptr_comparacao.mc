struct Nodo {
    int valor;
};

int main() {
    Nodo* ptr = nullptr;
    int resultado = 0;
    
    if (ptr == nullptr) {
        resultado = 1; 
    } else {
        resultado = 2;
    }
    return 0;
}