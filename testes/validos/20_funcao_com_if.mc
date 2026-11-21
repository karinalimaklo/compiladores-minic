int calcular(int x) {
    int y = x + 2;
    if( y == 3) {
        return y + 1;
    }
    return y;
}

int main() {
    return calcular(5);
}