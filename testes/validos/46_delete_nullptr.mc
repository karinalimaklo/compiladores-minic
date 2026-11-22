int main() {
    int* p = nullptr;

    delete p;
    delete nullptr;

    return 0;
}