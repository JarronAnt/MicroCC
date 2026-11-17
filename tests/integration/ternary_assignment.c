int main() {
    int x = 4;
    int y = 0 ? (x = 6) : (x = 7);
    return x + y;
}
