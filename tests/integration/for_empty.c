int main() {
    int x = 0;
    for (;;) {
        if (++x == 5)
            break;
    }
    return x;
}
