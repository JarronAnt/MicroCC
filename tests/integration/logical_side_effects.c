int main() {
    int x = 0;
    int y = (0 && (x = 3)) || (1 && (x = 8));
    return x + y;
}
