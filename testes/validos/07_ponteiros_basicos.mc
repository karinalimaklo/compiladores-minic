int main() {
    int x = 42;
    int* p = &x;
    int** ptr = &p;
    
    *p = 10;
    int y = **ptr + *p;
    return 0;
}
