int main() {
    int x = 10;
    {
        int x = 3;
        x += 2;
    }
    return x;
}
