int main() {
    int x = 10;
    {
        int x = 20; 
    }
    int resultado = x + 5; 
    return 0;
}